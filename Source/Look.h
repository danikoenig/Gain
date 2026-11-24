#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Colours.h"

// Schrift: Init wie bei den anderen Plugins die Standardschrift von JUCE (kein Schriftname gesetzt)
// Je Theme kann eine eigene Schrift gesetzt sein (Init und Colorblind: Standard)
inline juce::FontOptions gainFont (const GainColours::Theme& t, float size, bool bold = false)
{
    juce::FontOptions f (size, bold ? juce::Font::bold : juce::Font::plain);
    if (t.fontName[0] != 0) f = f.withName (t.fontName);
    return f;
}

// Look & Feel im Dani-Koenig-Design (wie Knack, One Shooter ...), Hochformat
class GainLookAndFeel : public juce::LookAndFeel_V4
{
public:
    GainLookAndFeel();
    void setTheme (const GainColours::Theme* t);
    const GainColours::Theme& getTheme() const { return *theme; }
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText (juce::Graphics&, juce::TextButton&, bool, bool) override;
    juce::Label* createSliderTextBox (juce::Slider&) override;

private:
    const GainColours::Theme* theme = &GainColours::themeFor (0);
};

// Meter: zwei Balken (links / rechts). Anzeige per Rechtsklick: RMS (Standard, trag), True Peak oder beides
// uebereinander (RMS dunkler, True Peak heller, mit Halten),
// Dreieck links = Zielpegel (ziehen), Skala in dB relativ zum Ziel (0 = Dreieck), Farben nach dem Zielpegel
class GainMeter : public juce::Component
{
public:
    void setLevels (const float* rms2, const float* peak2)   // links, rechts
    {
        for (int i = 0; i < 2; ++i) { rmsDb[i] = rms2[i]; peakDb[i] = peak2[i]; }
        repaint();
    }
    void setTheme (const GainColours::Theme* t) { th = t; repaint(); }
    void setMode (int m) { if (m != mode) { mode = m; repaint(); } }   // 0 RMS, 1 True Peak, 2 RMS + Peak
    std::function<void (int mode)> onModeChange;
    void setTarget (float t) { if (std::abs (t - targetDb) > 0.001f) { targetDb = t; repaint(); } }
    float getTarget() const { return targetDb; }

    // phase: 0 = Start, 1 = Ziehen, 2 = Ende
    std::function<void (int phase, float db)> onTargetDrag;

    // true, solange das Dreieck gezogen wird oder die Maus darueber steht (fuer die Wert-Anzeige)
    bool isTargetActive() const { return dragging || hoverTarget; }
    float getTargetY() const { return yForDb (targetDb); }

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseMove (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    float yForDb (float db) const;
    float dbForY (float y) const;
    juce::Rectangle<float> barArea() const;
    void drag (const juce::MouseEvent&, int phase);

    void showPresetMenu();
    void showModeMenu();
    bool nearTriangle (const juce::MouseEvent&) const;
    int mode = 0;
    const GainColours::Theme* th = &GainColours::themeFor (0);

    float rmsDb[2] = { -100.0f, -100.0f }, peakDb[2] = { -100.0f, -100.0f }, targetDb = -18.0f;
    bool dragging = false, hoverTarget = false;
};
