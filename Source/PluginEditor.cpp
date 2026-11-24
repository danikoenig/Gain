#include "PluginEditor.h"

using namespace juce;
using namespace GainColours;

GainEditor::GainEditor (GainProcessor& p) : AudioProcessorEditor (&p), processor (p)
{
    auto& apvts = processor.apvts;
    themeIndex = roundToInt (apvts.getRawParameterValue (ParamID::theme)->load());
    th = &themeFor (themeIndex);
    tint = th->blue;
    lnf.setTheme (th);
    meter.setTheme (th);
    setLookAndFeel (&lnf);

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

    meter.onModeChange = [this] (int m)
    {
        if (auto* par = processor.apvts.getParameter (ParamID::meterMode))
        {
            par->beginChangeGesture();
            par->setValueNotifyingHost (par->convertTo0to1 ((float) m));
            par->endChangeGesture();
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
    calBtn.setTooltip ("Listens for 5 s and sets GAIN so the average (RMS) level lands on the triangle");
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

void GainEditor::applyTheme (int index)
{
    themeIndex = index;
    th = &themeFor (index);
    lnf.setTheme (th);
    meter.setTheme (th);
    tint = th->blue;
    slowDb = -200.0f;
    sendLookAndFeelChange();   // Textfeld des Knobs neu aufbauen
    resized();
    repaint();
}

void GainEditor::showThemeMenu()
{
    PopupMenu m;
    m.addSectionHeader ("Theme");
    const auto names = themeNames();
    for (int i = 0; i < names.size(); ++i)
        m.addItem (i + 1, names[i], true, i == themeIndex);

    m.showMenuAsync (PopupMenu::Options().withTargetComponent (this).withMinimumWidth (200),
                     [safe = Component::SafePointer<GainEditor> (this)] (int result)
    {
        if (safe == nullptr || result <= 0) return;
        if (auto* par = safe->processor.apvts.getParameter (ParamID::theme))
        {
            par->beginChangeGesture();
            par->setValueNotifyingHost (par->convertTo0to1 ((float) (result - 1)));
            par->endChangeGesture();
        }
    });
}

void GainEditor::mouseDown (const MouseEvent& e)
{
    // Rechtsklick auf die Kopfzeile: Theme waehlen
    if (e.mods.isPopupMenu() && e.position.y < 72.0f * scale) showThemeMenu();
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

    gainKnob.setBounds (R (0, 74, 262, 300));
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
    const int wantTheme = roundToInt (processor.apvts.getRawParameterValue (ParamID::theme)->load());
    if (wantTheme != themeIndex) applyTheme (wantTheme);

    const float target = processor.apvts.getRawParameterValue (ParamID::target)->load();
    const float rms = e.rmsDb.load(), peak = e.peakDb.load();

    // Anzeigewerte, Farbe und Glow folgen dem RMS-Pegel relativ zum Zielpegel
    rmsShown = rms; peakShown = peak; targetShown = target;
    // Plugin-Farbe folgt einem langsam gemittelten Pegel (ca. 2 s), damit sie nicht im Takt flackert
    const float rmsIn = rms < -70.0f ? target - 20.0f : rms;   // Stille = blau; rms = echter RMS in dBFS
    if (slowDb < -150.0f) slowDb = rmsIn;
    slowDb += (rmsIn - slowDb) * 0.0165f;
    const float db = peak > -1.0f ? jmax (slowDb, -2.0f) : slowDb;   // echte Spitzen nahe 0 dBFS faerben trotzdem rot
    const Colour wanted = levelColour (*th, db, target);
    tint = tint.interpolatedWith (wanted, 0.25f);

    // Sweet Spot der letzten Calibration: beim Ueberfahren blitzt das Plugin kurz gruen, am Punkt bleibt es etwas heller
    {
        const float sweet = processor.sweetGainDb.load();
        const float gainNow = processor.apvts.getRawParameterValue (ParamID::gain)->load();
        bool atSweet = false;
        if (! std::isnan (sweet))
        {
            atSweet = std::abs (gainNow - sweet) < 0.15f;
            const bool crossed = sweetInit && (prevGain - sweet) * (gainNow - sweet) < 0.0f;
            if (sweetInit && (crossed || (atSweet && ! wasAtSweet)) && gainNow != prevGain) flash = 1.0f;
        }
        const float rest = atSweet ? 0.3f : 0.0f;
        flash = rest + (flash - rest) * 0.88f;
        wasAtSweet = atSweet;
        prevGain = gainNow;
        sweetInit = true;
        gainKnob.getProperties().set ("sweet", (double) (std::isnan (sweet) ? -1000.0f : sweet));
    }
    shownTint = tint.interpolatedWith (th->green, jlimit (0.0f, 1.0f, flash));

    const float wantGlow = db > -4.0f ? 0.95f : db > target + 2.0f ? 0.5f : db > target - 2.0f ? 0.4f : 0.2f;
    glow += (wantGlow - glow) * 0.2f;
    shownGlow = jmax (glow, flash * 0.95f);

    meter.setTarget (target);
    meter.setMode (roundToInt (processor.apvts.getRawParameterValue (ParamID::meterMode)->load()));
    const float rmsCh[2] = { e.rmsDbCh[0].load(), e.rmsDbCh[1].load() };
    const float pkCh[2] = { e.peakDbCh[0].load(), e.peakDbCh[1].load() };
    meter.setLevels (rmsCh, pkCh);

    gainKnob.getProperties().set ("tint", shownTint.toString());
    gainKnob.getProperties().set ("glow", (double) (shownGlow * th->glowScale));
    gainKnob.repaint();

    auto setupChoice = [&] (TextButton& b, const char* id, const StringArray& names)
    {
        const int v = roundToInt (processor.apvts.getRawParameterValue (id)->load());
        b.getProperties().set ("state", names[jlimit (0, 2, v)]);
        b.getProperties().set ("active", v != 0);
        b.getProperties().set ("tint", shownTint.toString());
        b.repaint();
    };
    {
        const bool on = processor.apvts.getRawParameterValue (ParamID::mono)->load() > 0.5f;
        monoBtn.getProperties().set ("state", on ? "ON" : "OFF");
        monoBtn.getProperties().set ("active", on);
        monoBtn.getProperties().set ("tint", shownTint.toString());
        monoBtn.repaint();
    }
    setupChoice (lowBtn, ParamID::lowCut, { "OFF", "25 Hz", "100 Hz" });
    setupChoice (highBtn, ParamID::highCut, { "OFF", "18 kHz", "10 kHz" });

    const bool cal = e.isCalibrating();
    int ticks = processor.noSignalTicks.load();
    if (ticks > 0) processor.noSignalTicks = --ticks;
    calBtn.getProperties().set ("calibrating", cal);
    calBtn.getProperties().set ("progress", (double) e.getCalProgress());
    calBtn.getProperties().set ("tint", shownTint.toString());
    int limTicks = processor.peakLimitTicks.load();
    if (limTicks > 0) processor.peakLimitTicks = --limTicks;
    calBtn.setButtonText (cal ? "LISTENING" : ticks > 0 ? "NO SIGNAL" : limTicks > 0 ? "PEAK LIMIT" : "CALIBRATE");
    calBtn.repaint();

    repaint (R (0, 84, designW, 300));
}

void GainEditor::paint (Graphics& g)
{
    GAIN_LOCALS (*th);
    const float gs = th->glowScale;
    const auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (ColourGradient (bgTop, 0.0f, 0.0f, bgBottom, 0.0f, bounds.getBottom(), false));
    g.fillAll();

    // Schein hinter dem Knob in der aktuellen Pegelfarbe
    const auto c = Point<float> (134.0f * scale, 224.0f * scale);
    const float rad = (150.0f + 260.0f * shownGlow) * scale;
    g.setGradientFill (ColourGradient (shownTint.withAlpha ((0.03f + 0.36f * shownGlow) * gs), c.x, c.y,
                                       shownTint.withAlpha (0.0f), c.x + rad, c.y, true));
    g.fillAll();

    // Kopfzeile wie bei den anderen Plugins: Titel gross, klein daneben "by Dani Koenig . v1.x" auf der Grundlinie
    const String byText = String::fromUTF8 ("by Dani Koenig  \xc2\xb7  v") + JucePlugin_VersionString;
    {
        const String title = "GAIN";
        const float titleSize = 60.0f * scale, spacing = th->titleSpacing * scale, smallSize = 11.0f * scale, gapW = 14.0f * scale;
        Font big (gainFont (titleSize, true)), small (gainFont (smallSize));
        float titleW = 0.0f;
        for (int i = 0; i < title.length(); ++i)
            titleW += big.getStringWidthFloat (String::charToString (title[i])) + spacing;
        titleW -= spacing;
        const float smallW = small.getStringWidthFloat (byText);
        float x = ((float) getWidth() - (titleW + gapW + smallW)) * 0.5f;

        g.setColour (text);
        g.setFont (big);
        for (int i = 0; i < title.length(); ++i)
        {
            const auto ch = String::charToString (title[i]);
            const float gw = big.getStringWidthFloat (ch);
            g.drawText (ch, Rectangle<float> (x, 8.0f * scale, gw + 4.0f, 62.0f * scale), Justification::centredLeft);
            x += gw + spacing;
        }
        x = x - spacing + gapW;
        g.setColour (textDim);
        g.setFont (small);
        g.drawText (byText, Rectangle<float> (x, 44.0f * scale, smallW + 6.0f, 16.0f * scale), Justification::centredLeft);
    }

    // Trennlinie
    g.setColour (cardBorder);
    g.drawHorizontalLine (roundToInt (380.0f * scale), 16.0f * scale, (designW - 16.0f) * scale);

}

void GainEditor::paintOverChildren (Graphics& g)
{
    GAIN_LOCALS (*th);
    const float gs = th->glowScale;
    if (meter.isTargetActive())
    {
        const float db = targetShown;
        const auto mb = meter.getBounds().toFloat();
        const float cy = mb.getY() + meter.getTargetY();
        const String line1 = "TARGET  " + String (db, 1) + " dBFS RMS";
        Font f1 (gainFont (12.0f * scale, true));
        const float w = f1.getStringWidthFloat (line1) + 20.0f * scale, h = 24.0f * scale;
        auto box = Rectangle<float> (mb.getX() - 18.0f * scale - w, cy - h * 0.5f, w, h);
        g.setColour (inset.withAlpha (0.96f));
        g.fillRoundedRectangle (box, 6.0f * scale);
        g.setColour (shownTint.withAlpha (0.9f));
        g.drawRoundedRectangle (box, 6.0f * scale, 1.2f);
        g.setColour (text);
        g.setFont (f1);
        g.drawText (line1, box, Justification::centred);
    }

    auto r = getLocalBounds().toFloat().reduced (1.0f);
    if (th->frame.getAlpha() > 0)
    {
        // fester Rahmen wie bei den anderen Plugins, der Pegel-Schein sitzt knapp innen
        g.setColour (th->frame.withAlpha (0.9f));
        g.drawRoundedRectangle (r, th->radius * scale, 2.0f);
        r = r.reduced (3.5f * scale);
    }
    g.setColour (shownTint.withAlpha ((0.08f + 0.6f * shownGlow * shownGlow) * jmax (gs, 0.5f)));
    g.drawRoundedRectangle (r, th->radius * scale, 1.5f);
}
