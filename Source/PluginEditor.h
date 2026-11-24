#pragma once

#include "PluginProcessor.h"
#include "Look.h"
#include <juce_audio_utils/juce_audio_utils.h>

class GainEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit GainEditor (GainProcessor&);
    ~GainEditor() override;

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    juce::Rectangle<int> R (float x, float y, float w, float h) const;
    void cycleChoice (const char* id);

    static constexpr float designW = 360.0f, designH = 452.0f;

    GainProcessor& processor;
    GainLookAndFeel lnf;

    juce::Slider gainKnob;
    GainMeter meter;
    juce::TextButton monoBtn, lowBtn, highBtn, calBtn { "CALIBRATE" };

    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SA> gainAtt;

    juce::Colour tint = GainColours::blue;
    float slowDb = -200.0f, glow = 0.2f, scale = 1.0f, rmsShown = -100.0f, peakShown = -100.0f, targetShown = -18.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GainEditor)
};
