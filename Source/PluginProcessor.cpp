#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

AudioProcessorValueTreeState::ParameterLayout GainProcessor::createLayout()
{
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::gain, 1 }, "Gain",
                    NormalisableRange<float> (-GainDSP::maxGainDb, GainDSP::maxGainDb, 0.1f), 0.0f,
                    AudioParameterFloatAttributes()
                        .withStringFromValueFunction ([] (float v, int) { return (v > 0.04f ? "+" : "") + String (v, 1) + " dB"; })
                        .withValueFromStringFunction ([] (const String& t) { return t.getFloatValue(); })));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::lowCut, 1 }, "Low Cut",
                    StringArray { "Off", "25 Hz", "100 Hz" }, 0));
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::highCut, 1 }, "High Cut",
                    StringArray { "Off", "18 kHz", "10 kHz" }, 0));
    p.push_back (std::make_unique<AudioParameterFloat> (ParameterID { ParamID::target, 1 }, "Target Level",
                    NormalisableRange<float> (-36.0f, -6.0f, 0.5f), -18.0f,
                    AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int) { return String (v, 1) + " dBFS"; })));

    p.push_back (std::make_unique<AudioParameterBool> (ParameterID { ParamID::mono, 1 }, "Mono", false));
    // Anzeige: 0 = RMS (Standard), 1 = True Peak, 2 = RMS + Peak
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::meterMode, 1 }, "Meter Mode",
                    StringArray { "RMS", "True Peak", "RMS + Peak" }, 0));
    // Aussehen: 0 JUCE (Original), 1 Grey, 2 Dark, 3 Colour-Blind
    p.push_back (std::make_unique<AudioParameterChoice> (ParameterID { ParamID::theme, 1 }, "Theme",
                    StringArray { "JUCE", "Grey", "Dark", "Colour-Blind" }, 0));

    return { p.begin(), p.end() };
}

GainProcessor::GainProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", AudioChannelSet::stereo(), true)
                                       .withOutput ("Output", AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "GAIN", createLayout())
{
    pGain   = apvts.getRawParameterValue (ParamID::gain);
    pLow    = apvts.getRawParameterValue (ParamID::lowCut);
    pHigh   = apvts.getRawParameterValue (ParamID::highCut);
    pTarget = apvts.getRawParameterValue (ParamID::target);
    pMono   = apvts.getRawParameterValue (ParamID::mono);
}

bool GainProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    const auto o = l.getMainOutputChannelSet();
    return (o == AudioChannelSet::stereo() || o == AudioChannelSet::mono()) && o == l.getMainInputChannelSet();
}

void GainProcessor::prepareToPlay (double sampleRate, int)
{
    engine.prepare (sampleRate);
}

void GainProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer&)
{
    ScopedNoDenormals noDenormals;
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, buffer.getNumSamples());

    GainParams p;
    p.gainDb   = pGain->load();
    p.lowCut   = (int) pLow->load();
    p.highCut  = (int) pHigh->load();
    p.targetDb = pTarget->load();
    p.mono     = pMono->load() > 0.5f;

    engine.process (buffer.getArrayOfWritePointers(), jmin (buffer.getNumChannels(), getTotalNumInputChannels()),
                    buffer.getNumSamples(), p);

    float db;
    if (engine.popCalibrationResult (db))
    {
        pendingGainDb = db;
        pendingFailed = false;
        pendingLimited = engine.popCalibrationLimited();
        triggerAsyncUpdate();
    }
    else if (engine.popCalibrationFailed())
    {
        pendingFailed = true;
        triggerAsyncUpdate();
    }
}

void GainProcessor::handleAsyncUpdate()
{
    if (pendingFailed.exchange (false))
    {
        noSignalTicks = 60;
        return;
    }

    if (pendingLimited.exchange (false)) peakLimitTicks = 75;   // ca. 2.5 s Hinweis am Button

    // Calibration fertig: Gain-Regler faehrt animiert auf den gemessenen Wert
    if (auto* par = apvts.getParameter (ParamID::gain))
    {
        animFrom = par->convertFrom0to1 (par->getValue());
        animTo = pendingGainDb.load();
        sweetGainDb = std::round (animTo * 10.0f) / 10.0f;   // so wie der Regler den Wert speichert (0.1-Schritte)
        animStep = 0;
        par->beginChangeGesture();
        startTimerHz (60);
    }
}

void GainProcessor::timerCallback()
{
    auto* par = apvts.getParameter (ParamID::gain);
    if (par == nullptr) { stopTimer(); return; }

    ++animStep;
    const float t = jmin (1.0f, (float) animStep / (float) animSteps);
    const float e = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);   // ease-out
    par->setValueNotifyingHost (par->convertTo0to1 (animFrom + (animTo - animFrom) * e));
    if (t >= 1.0f)
    {
        par->endChangeGesture();
        stopTimer();
    }
}

AudioProcessorEditor* GainProcessor::createEditor() { return new GainEditor (*this); }

void GainProcessor::getStateInformation (MemoryBlock& dest)
{
    apvts.state.setProperty ("sweetGain", sweetGainDb.load(), nullptr);
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void GainProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (ValueTree::fromXml (*xml));
            if (apvts.state.hasProperty ("sweetGain"))
                sweetGainDb = (float) (double) apvts.state.getProperty ("sweetGain");
        }
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new GainProcessor(); }
