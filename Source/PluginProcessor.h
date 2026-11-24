#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "Engine.h"

namespace ParamID
{
    inline constexpr const char* gain    = "gain";
    inline constexpr const char* lowCut  = "lowcut";
    inline constexpr const char* highCut = "highcut";
    inline constexpr const char* target  = "target";
    inline constexpr const char* mono    = "mono";
}

class GainProcessor : public juce::AudioProcessor, private juce::AsyncUpdater, private juce::Timer
{
public:
    GainProcessor();
    ~GainProcessor() override { cancelPendingUpdate(); stopTimer(); }

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "GAIN"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    GainEngine& getEngine() { return engine; }
    juce::AudioProcessorValueTreeState apvts;

    // Meldung fuer die Oberflaeche ("NO SIGNAL" nach fehlgeschlagener Calibration)
    std::atomic<int> noSignalTicks { 0 };
    std::atomic<int> peakLimitTicks { 0 };   // Calibration wurde durch die Peak-Sicherung begrenzt

private:
    void handleAsyncUpdate() override;
    void timerCallback() override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    GainEngine engine;
    std::atomic<float>* pGain = nullptr; std::atomic<float>* pLow = nullptr;
    std::atomic<float>* pHigh = nullptr; std::atomic<float>* pTarget = nullptr; std::atomic<float>* pMono = nullptr;

    // Animation: Gain-Knob dreht sich nach der Calibration zum neuen Wert
    std::atomic<float> pendingGainDb { 0.0f };
    std::atomic<bool> pendingFailed { false }, pendingLimited { false };
    float animFrom = 0.0f, animTo = 0.0f;
    int animStep = 0;
    static constexpr int animSteps = 24;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainProcessor)
};
