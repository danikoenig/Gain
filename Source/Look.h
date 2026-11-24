#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Colours.h"

// Schrift: Helvetica (Schweizer Klassiker)
inline juce::FontOptions gainFont (float size, bool bold = false)
{
    return juce::FontOptions ("Helvetica Neue", size, bold ? juce::Font::bold : juce::Font::plain);
}

// Look & Feel im Dani-Koenig-Design (wie Knack, One Shooter ...), Hochformat
class GainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GainLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;
    juce::Label* createSliderTextBox (juce::Slider&) override;
};

// Meter wie bei Ableton: zwei Balken (links / rechts), jeder mit VU (Ballistik 300 ms, volle Farbe)
// und True Peak (gedaempft darueber, mit Halten) uebereinander,
// Dreieck links = Zielpegel = 0 VU (ziehen), Skala in VU relativ zum Ziel, Farben nach dem Zielpegel
class GainMeter : public juce::Component
{
public:
    void setLevels (const float* rms2, const float* peak2)   // links, rechts
    {
        for (int i = 0; i < 2; ++i) { rmsDb[i] = rms2[i]; peakDb[i] = peak2[i]; }
        repaint();
    }
    void setTarget (float t) { if (std::abs (t - targetDb) > 0.001f) { targetDb = t; repaint(); } }
    float getTarget() const { return targetDb; }

    // phase: 0 = Start, 1 = Ziehen, 2 = Ende
    std::function<void (int phase, float db)> onTargetDrag;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    float yForDb (float db) const;
    float dbForY (float y) const;
    juce::Rectangle<float> barArea() const;
    void drag (const juce::MouseEvent&, int phase);

    float rmsDb[2] = { -100.0f, -100.0f }, peakDb[2] = { -100.0f, -100.0f }, targetDb = -18.0f;
};
