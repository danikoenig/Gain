#include "PluginEditor.h"

using namespace juce;
using namespace GainColours;

GainEditor::GainEditor (GainProcessor& p) : AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lnf);
    auto& apvts = processor.apvts;

    gainKnob.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
    gainKnob.setTextBoxStyle (Slider::TextBoxBelow, false, 120, 22);
    gainKnob.setRotaryParameters (-0.75f * MathConstants<float>::pi, 0.75f * MathConstants<float>::pi, true);
    gainKnob.setMouseDragSensitivity (260);
    gainKnob.setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    gainKnob.setTooltip ("Gain +/-35 dB (double-click = 0 dB)");
    addAndMakeVisible (gainKnob);
    gainAtt = std::make_unique<SA> (apvts, ParamID::gain, gainKnob);
    gainKnob.setDoubleClickReturnValue (true, 0.0);

    addAndMakeVisible (meter);
    meter.onTargetDrag = [this] (int phase, float db)
    {
        if (auto* par = processor.apvts.getParameter (ParamID::target))
        {
            if (phase == 0) par->beginChangeGesture();
            par->setValueNotifyingHost (par->convertTo0to1 (db));
            if (phase == 2) par->endChangeGesture();
        }
    };

    monoBtn.getProperties().set ("label", "MONO");
    lowBtn.getProperties().set ("label", "LOW CUT");
    highBtn.getProperties().set ("label", "HIGH CUT");
    monoBtn.setTooltip ("Mono: sums left and right to (L+R)/2, before the meter and the gain");
    lowBtn.setTooltip ("Low cut: off / 25 Hz / 100 Hz (12 dB/oct)");
    highBtn.setTooltip ("High cut: off / 18 kHz / 10 kHz (12 dB/oct)");
    monoBtn.onClick = [this]
    {
        if (auto* par = processor.apvts.getParameter (ParamID::mono))
        {
            par->beginChangeGesture();
            par->setValueNotifyingHost (par->getValue() > 0.5f ? 0.0f : 1.0f);
            par->endChangeGesture();
        }
    };
    lowBtn.onClick  = [this] { cycleChoice (ParamID::lowCut); };
    highBtn.onClick = [this] { cycleChoice (ParamID::highCut); };
    calBtn.setTooltip ("Listens for 5 s and sets GAIN so the VU level lands on 0 (the triangle)");
    calBtn.onClick = [this]
    {
        auto& e = processor.getEngine();
        if (e.isCalibrating()) e.cancelCalibration(); else e.startCalibration();
    };
    addAndMakeVisible (monoBtn);
    addAndMakeVisible (lowBtn);
    addAndMakeVisible (highBtn);
    addAndMakeVisible (calBtn);

    setResizable (true, true);
    getConstrainer()->setFixedAspectRatio ((double) designW / (double) designH);
    setResizeLimits (270, (int) (270.0f * designH / designW), 720, (int) (720.0f * designH / designW));
    setSize ((int) designW, (int) designH);

    startTimerHz (30);
}

GainEditor::~GainEditor()
{
    setLookAndFeel (nullptr);
}

void GainEditor::cycleChoice (const char* id)
{
    if (auto* par = processor.apvts.getParameter (id))
    {
        const int cur = roundToInt (par->convertFrom0to1 (par->getValue()));
        par->beginChangeGesture();
        par->setValueNotifyingHost (par->convertTo0to1 ((float) ((cur + 1) % 3)));
        par->endChangeGesture();
    }
}

Rectangle<int> GainEditor::R (float x, float y, float w, float h) const
{
    return { roundToInt (x * scale), roundToInt (y * scale), roundToInt (w * scale), roundToInt (h * scale) };
}

void GainEditor::resized()
{
    scale = (float) getWidth() / designW;

    gainKnob.setBounds (R (10, 84, 248, 280));
    gainKnob.setTextBoxStyle (Slider::TextBoxBelow, false, roundToInt (120 * scale), roundToInt (24 * scale));
    meter.setBounds (R (268, 84, 76, 276));

    monoBtn.setBounds (R (16, 392, 76, 44));
    lowBtn.setBounds  (R (100, 392, 76, 44));
    highBtn.setBounds (R (184, 392, 76, 44));
    calBtn.setBounds  (R (268, 392, 76, 44));
}

void GainEditor::timerCallback()
{
    auto& e = processor.getEngine();
    const float target = processor.apvts.getRawParameterValue (ParamID::target)->load();
    const float rms = e.rmsDb.load(), peak = e.peakDb.load();

    // Anzeigewerte, Farbe und Glow folgen dem RMS-Pegel relativ zum Zielpegel
    rmsShown = rms; peakShown = peak; targetShown = target;
    const float db = rms < -70.0f ? target - 20.0f : rms;   // Stille = blau; rms = normaler RMS in dBFS, 0 VU = Ziel
    const Colour wanted = levelColour (db, target);
    tint = tint.interpolatedWith (wanted, 0.25f);
    const float wantGlow = db > -4.0f ? 0.95f : db > target + 2.0f ? 0.5f : db > target - 2.0f ? 0.4f : 0.2f;
    glow += (wantGlow - glow) * 0.2f;

    meter.setTarget (target);
    const float rmsCh[2] = { e.rmsDbCh[0].load(), e.rmsDbCh[1].load() };
    const float pkCh[2] = { e.peakDbCh[0].load(), e.peakDbCh[1].load() };
    meter.setLevels (rmsCh, pkCh);

    gainKnob.getProperties().set ("tint", tint.toString());
    gainKnob.getProperties().set ("glow", (double) glow);
    gainKnob.repaint();

    auto setupChoice = [&] (TextButton& b, const char* id, const StringArray& names)
    {
        const int v = roundToInt (processor.apvts.getRawParameterValue (id)->load());
        b.getProperties().set ("state", names[jlimit (0, 2, v)]);
        b.getProperties().set ("active", v != 0);
        b.getProperties().set ("tint", tint.toString());
        b.repaint();
    };
    {
        const bool on = processor.apvts.getRawParameterValue (ParamID::mono)->load() > 0.5f;
        monoBtn.getProperties().set ("state", on ? "ON" : "OFF");
        monoBtn.getProperties().set ("active", on);
        monoBtn.getProperties().set ("tint", tint.toString());
        monoBtn.repaint();
    }
    setupChoice (lowBtn, ParamID::lowCut, { "OFF", "25 Hz", "100 Hz" });
    setupChoice (highBtn, ParamID::highCut, { "OFF", "18 kHz", "10 kHz" });

    const bool cal = e.isCalibrating();
    int ticks = processor.noSignalTicks.load();
    if (ticks > 0) processor.noSignalTicks = --ticks;
    calBtn.getProperties().set ("calibrating", cal);
    calBtn.getProperties().set ("progress", (double) e.getCalProgress());
    calBtn.getProperties().set ("tint", tint.toString());
    calBtn.setButtonText (cal ? "LISTENING" : ticks > 0 ? "NO SIGNAL" : "CALIBRATE");
    calBtn.repaint();

    repaint (R (0, 84, designW, 300));
}

void GainEditor::paint (Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (ColourGradient (bgTop, 0.0f, 0.0f, bgBottom, 0.0f, bounds.getBottom(), false));
    g.fillAll();

    // Schein hinter dem Knob in der aktuellen Pegelfarbe
    const auto c = Point<float> (134.0f * scale, 224.0f * scale);
    const float rad = (150.0f + 260.0f * glow) * scale;
    g.setGradientFill (ColourGradient (tint.withAlpha (0.03f + 0.36f * glow), c.x, c.y,
                                       tint.withAlpha (0.0f), c.x + rad, c.y, true));
    g.fillAll();

    // Titel: gross, mittig
    g.setColour (text);
    g.setFont (gainFont (60.0f * scale, true));
    {
        const String title = "GAIN";
        const float spacing = 12.0f * scale;
        float wTot = 0.0f;
        for (int i = 0; i < title.length(); ++i)
            wTot += g.getCurrentFont().getStringWidthFloat (String::charToString (title[i])) + spacing;
        wTot -= spacing;
        float x = ((float) getWidth() - wTot) * 0.5f;
        for (int i = 0; i < title.length(); ++i)
        {
            const auto s = String::charToString (title[i]);
            const float gw = g.getCurrentFont().getStringWidthFloat (s);
            g.drawText (s, Rectangle<float> (x, 8.0f * scale, gw + 4.0f, 68.0f * scale), Justification::centredLeft);
            x += gw + spacing;
        }
    }

    // Trennlinie
    g.setColour (cardBorder);
    g.drawHorizontalLine (roundToInt (380.0f * scale), 16.0f * scale, (designW - 16.0f) * scale);

    g.setColour (textDim);
    g.setFont (gainFont (9.5f * scale, true));
    g.drawText (String::fromUTF8 ("by Dani Koenig  \xc2\xb7  v") + JucePlugin_VersionString, R (0, 448, designW, 12).toFloat(), Justification::centred);
}

void GainEditor::paintOverChildren (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (tint.withAlpha (0.08f + 0.6f * glow * glow));
    g.drawRoundedRectangle (r, 6.0f * scale, 1.5f);
}
