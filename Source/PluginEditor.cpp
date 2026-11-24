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

    lowBtn.getProperties().set ("label", "LOW CUT");
    highBtn.getProperties().set ("label", "HIGH CUT");
    lowBtn.setTooltip ("Low cut: off / 25 Hz / 100 Hz (12 dB/oct)");
    highBtn.setTooltip ("High cut: off / 18 kHz / 10 kHz (12 dB/oct)");
    lowBtn.onClick  = [this] { cycleChoice (ParamID::lowCut); };
    highBtn.onClick = [this] { cycleChoice (ParamID::highCut); };
    calBtn.setTooltip ("Listens for 5 s and sets GAIN so the VU level lands on 0 (the triangle)");
    calBtn.onClick = [this]
    {
        auto& e = processor.getEngine();
        if (e.isCalibrating()) e.cancelCalibration(); else e.startCalibration();
    };
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

    lowBtn.setBounds  (R (16, 392, 102, 44));
    highBtn.setBounds (R (129, 392, 102, 44));
    calBtn.setBounds  (R (242, 392, 102, 44));
}

void GainEditor::timerCallback()
{
    auto& e = processor.getEngine();
    const float target = processor.apvts.getRawParameterValue (ParamID::target)->load();
    const float rms = e.rmsDb.load(), peak = e.peakDb.load();

    // Anzeigewerte, Farbe und Glow folgen dem RMS-Pegel relativ zum Zielpegel
    rmsShown = rms; peakShown = peak; targetShown = target;
    const float db = rms < -70.0f ? target - 20.0f : rms;   // Stille = blau; rms = Pegel auf AES-17-Skala (VU 0 = Ziel)
    const Colour wanted = levelColour (db, target);
    tint = tint.interpolatedWith (wanted, 0.25f);
    const float wantGlow = db > -4.0f ? 0.95f : db > target + 2.0f ? 0.5f : db > target - 2.0f ? 0.4f : 0.2f;
    glow += (wantGlow - glow) * 0.2f;

    meter.setTarget (target);
    meter.setLevels (rms, peak);

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
    setupChoice (lowBtn, ParamID::lowCut, { "OFF", "25 Hz", "100 Hz" });
    setupChoice (highBtn, ParamID::highCut, { "OFF", "18 kHz", "10 kHz" });

    const bool cal = e.isCalibrating();
    int ticks = processor.noSignalTicks.load();
    if (ticks > 0) processor.noSignalTicks = --ticks;
    calBtn.getProperties().set ("calibrating", cal);
    calBtn.getProperties().set ("progress", (double) e.getCalProgress());
    calBtn.getProperties().set ("tint", tint.toString());
    calBtn.setButtonText (cal ? "LISTENING..." : ticks > 0 ? "NO SIGNAL" : "CALIBRATE");
    calBtn.repaint();

    repaint (R (0, 0, designW, 84));   // Pegelzahlen
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

    // Titel
    g.setColour (text);
    g.setFont (gainFont (44.0f * scale, true));
    {
        const String title = "GAIN";
        const float spacing = 7.0f * scale;
        float x = 18.0f * scale;
        for (int i = 0; i < title.length(); ++i)
        {
            const auto s = String::charToString (title[i]);
            const float gw = g.getCurrentFont().getStringWidthFloat (s);
            g.drawText (s, Rectangle<float> (x, 9.0f * scale, gw + 4.0f, 50.0f * scale), Justification::centredLeft);
            x += gw + spacing;
        }
    }

    // Pegelzahlen oben rechts
    auto num = [] (float v) { return v < -90.0f ? String ("--") : String (v, 1); };
    const String vuText = rmsShown < -90.0f ? String ("--") : (rmsShown - targetShown > 0.04f ? "+" : "") + String (rmsShown - targetShown, 1);
    g.setFont (gainFont (9.5f * scale, true));
    g.setColour (textDim);
    g.drawText ("VU", R (190, 14, 40, 12).toFloat(), Justification::centredRight);
    g.drawText ("TRUE PEAK dBFS", R (150, 40, 110, 12).toFloat(), Justification::centredRight);
    g.setFont (gainFont (13.0f * scale, true));
    g.setColour (text);
    g.drawText (vuText, R (262, 12, 82, 16).toFloat(), Justification::centredRight);
    g.drawText (num (peakShown), R (262, 38, 82, 16).toFloat(), Justification::centredRight);

    g.setColour (textDim);
    g.setFont (gainFont (10.5f * scale, true));
    g.drawText ("TARGET " + String (targetShown, 1) + " dBFS", R (200, 366, 144, 14).toFloat(), Justification::centredRight);

    // Trennlinie
    g.setColour (cardBorder);
    g.drawHorizontalLine (roundToInt (380.0f * scale), 16.0f * scale, (designW - 16.0f) * scale);

    g.setColour (textDim);
    g.setFont (gainFont (9.5f * scale, true));
    g.drawText ("by Dani Koenig", R (0, 448, designW, 12).toFloat(), Justification::centred);
}

void GainEditor::paintOverChildren (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.0f);
    g.setColour (tint.withAlpha (0.08f + 0.6f * glow * glow));
    g.drawRoundedRectangle (r, 6.0f * scale, 1.5f);
}
