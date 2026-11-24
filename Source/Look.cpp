#include "Look.h"
#include "Engine.h"
#include <cmath>

using namespace GainColours;
using namespace juce;

GainLookAndFeel::GainLookAndFeel()
{
    setDefaultSansSerifTypefaceName ("Helvetica Neue");
    setColour (Slider::textBoxTextColourId, text);
    setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
    setColour (Slider::textBoxBackgroundColourId, Colours::transparentBlack);
    setColour (Slider::textBoxHighlightColourId, blue.withAlpha (0.4f));
    setColour (Label::textColourId, text);
    setColour (TextEditor::highlightColourId, blue.withAlpha (0.35f));
    setColour (CaretComponent::caretColourId, text);
    setColour (TooltipWindow::backgroundColourId, card);
    setColour (TooltipWindow::textColourId, text);
}

Label* GainLookAndFeel::createSliderTextBox (Slider& s)
{
    auto* l = LookAndFeel_V4::createSliderTextBox (s);
    l->setFont (gainFont (15.0f, true));
    l->setJustificationType (Justification::centred);
    l->setColour (Label::textColourId, text);
    l->setColour (Label::backgroundColourId, Colours::transparentBlack);
    l->setColour (Label::outlineColourId, Colours::transparentBlack);
    l->setColour (TextEditor::backgroundColourId, inset);
    l->setColour (TextEditor::textColourId, text);
    l->setColour (TextEditor::outlineColourId, textDim);
    return l;
}

void GainLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                        float startAngle, float endAngle, Slider& s)
{
    const float glow = (float) (double) s.getProperties().getWithDefault ("glow", 0.2);
    const Colour col = Colour::fromString (s.getProperties().getWithDefault ("tint", blue.toString()).toString());
    const bool hover = s.isMouseOverOrDragging();

    auto bounds = Rectangle<int> (x, y, w, h).toFloat();
    const float size = jmin (bounds.getWidth(), bounds.getHeight()) - 16.0f;
    auto r = Rectangle<float> (size, size).withCentre (bounds.getCentre());
    const auto c = r.getCentre();
    const float radius = size * 0.5f;
    const float lineW = jmax (4.0f, size * 0.05f);
    const float arcR = radius - lineW * 0.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    const float zeroAngle = startAngle + 0.5f * (endAngle - startAngle);   // 0 dB = oben

    // Halo
    for (int i = 0; i < 5; ++i)
    {
        g.setColour (col.withAlpha ((0.04f + 0.5f * glow) * (0.22f - (float) i * 0.04f)));
        g.drawEllipse (r.expanded ((float) i * size * 0.022f + 2.0f), lineW * 0.9f);
    }

    // Track
    Path track;
    track.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (knobTrack);
    g.strokePath (track, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));

    // Wertring: von 0 dB (oben) aus in beide Richtungen
    if (std::abs (angle - zeroAngle) > 0.01f)
    {
        Path val;
        val.addCentredArc (c.x, c.y, arcR, arcR, 0.0f, jmin (zeroAngle, angle), jmax (zeroAngle, angle), true);
        g.setColour (col.withAlpha (0.18f + 0.3f * glow));
        g.strokePath (val, PathStrokeType (lineW * 3.2f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (col.withAlpha (0.25f + 0.3f * glow));
        g.strokePath (val, PathStrokeType (lineW * 2.0f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setColour (col);
        g.strokePath (val, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
    }

    // 0-dB-Strich oben
    g.setColour (text.withAlpha (0.9f));
    g.drawLine (c.x, r.getY() - 6.0f, c.x, r.getY() + lineW + 2.0f, 2.0f);

    // Koerper
    const float bodyR = arcR - lineW * 1.7f;
    auto body = Rectangle<float> (bodyR * 2.0f, bodyR * 2.0f).withCentre (c);
    g.setGradientFill (ColourGradient (knobBody.brighter (hover ? 0.3f : 0.18f), body.getX(), body.getY(),
                                       knobBody.darker (0.45f), body.getRight(), body.getBottom(), false));
    g.fillEllipse (body);
    g.setGradientFill (ColourGradient (col.withAlpha (0.0f), c.x, c.y,
                                       col.withAlpha (jmin (0.8f, 0.03f + 0.55f * glow)), c.x + bodyR, c.y, true));
    g.fillEllipse (body);
    g.setColour (Colours::white.withAlpha (0.07f));
    g.drawEllipse (body.reduced (0.5f), 1.0f);

    // Zeiger
    const float px = c.x + std::sin (angle) * bodyR * 0.80f, py = c.y - std::cos (angle) * bodyR * 0.80f;
    const float qx = c.x + std::sin (angle) * bodyR * 0.30f, qy = c.y - std::cos (angle) * bodyR * 0.30f;
    g.setColour (col.withAlpha (0.35f + 0.3f * glow));
    g.drawLine (qx, qy, px, py, 8.0f);
    g.setColour (text);
    g.drawLine (qx, qy, px, py, 3.6f);
}

void GainLookAndFeel::drawButtonBackground (Graphics& g, Button& b, const Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool calibrating = (bool) b.getProperties().getWithDefault ("calibrating", false);
    const bool active = (bool) b.getProperties().getWithDefault ("active", false);
    const float progress = (float) (double) b.getProperties().getWithDefault ("progress", 0.0);
    const Colour col = Colour::fromString (b.getProperties().getWithDefault ("tint", blue.toString()).toString());

    g.setColour (down ? cardBorder.brighter (0.2f) : highlighted ? cardBorder : inset);
    g.fillRoundedRectangle (r, 6.0f);
    if (active)
    {
        g.setColour (col.withAlpha (0.14f));
        g.fillRoundedRectangle (r, 6.0f);
    }
    if (calibrating)
    {
        g.setColour (col.withAlpha (0.35f));
        g.fillRoundedRectangle (r.withWidth (jmax (12.0f, r.getWidth() * progress)), 6.0f);
    }
    g.setColour (calibrating || active ? col : cardBorder.brighter (0.15f));
    g.drawRoundedRectangle (r, 6.0f, 1.2f);
}

void GainLookAndFeel::drawButtonText (Graphics& g, TextButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const bool active = (bool) b.getProperties().getWithDefault ("active", false);
    const float s = (float) b.getHeight() / 44.0f;
    const auto label = b.getProperties().getWithDefault ("label", "").toString();
    const auto state = b.getProperties().getWithDefault ("state", "").toString();

    if (label.isEmpty())
    {
        g.setColour (text.withAlpha (0.92f));
        g.setFont (gainFont (13.0f * s, true));
        g.drawText (b.getButtonText(), r, Justification::centred);
        return;
    }
    g.setColour (textDim);
    g.setFont (gainFont (9.5f * s, true));
    g.drawText (label, r.withTrimmedTop (6.0f * s).withHeight (12.0f * s), Justification::centred);
    g.setColour (active ? text : textDim.brighter (0.2f));
    g.setFont (gainFont (15.0f * s, true));
    g.drawText (state, r.withTrimmedTop (19.0f * s).withHeight (19.0f * s), Justification::centred);
}

//==============================================================================
Rectangle<float> GainMeter::barArea() const
{
    const float s = (float) getWidth() / 76.0f;
    return { 16.0f * s, 6.0f * s, 34.0f * s, (float) getHeight() - 12.0f * s };
}

float GainMeter::yForDb (float db) const
{
    const auto a = barArea();
    const float t = (GainDSP::meterMaxDb - jlimit (GainDSP::meterMinDb, GainDSP::meterMaxDb, db))
                    / (GainDSP::meterMaxDb - GainDSP::meterMinDb);
    return a.getY() + t * a.getHeight();
}

float GainMeter::dbForY (float y) const
{
    const auto a = barArea();
    const float t = jlimit (0.0f, 1.0f, (y - a.getY()) / a.getHeight());
    return GainDSP::meterMaxDb - t * (GainDSP::meterMaxDb - GainDSP::meterMinDb);
}

void GainMeter::paint (Graphics& g)
{
    const float s = (float) getWidth() / 76.0f;
    auto a = barArea();
    const float rmsW = 22.0f * s, gap = 4.0f * s, pkW = 8.0f * s;
    auto rmsBar = Rectangle<float> (a.getX(), a.getY(), rmsW, a.getHeight());
    auto pkBar = Rectangle<float> (rmsBar.getRight() + gap, a.getY(), pkW, a.getHeight());

    // Hintergrund
    g.setColour (inset);
    g.fillRoundedRectangle (Rectangle<float> (rmsBar.getX() - 2.0f * s, a.getY() - 2.0f * s,
                                              pkBar.getRight() - rmsBar.getX() + 4.0f * s, a.getHeight() + 4.0f * s), 4.0f * s);

    auto drawBar = [&] (Rectangle<float> bar, float levelDb)
    {
        const float yLevel = yForDb (levelDb);
        const float step = jmax (2.0f, 2.0f * s);
        for (float y = bar.getBottom() - step; y >= bar.getY() - 0.01f; y -= step)
        {
            const float db = dbForY (y + step * 0.5f);
            const bool lit = y + step * 0.5f >= yLevel && levelDb > GainDSP::meterMinDb;
            g.setColour (levelColour (db, targetDb).withAlpha (lit ? 1.0f : 0.13f));
            g.fillRect (bar.getX(), y, bar.getWidth(), step - 0.6f);
        }
    };
    drawBar (rmsBar, rmsDb);
    drawBar (pkBar, peakDb);

    // Skala
    g.setFont (gainFont (8.5f * s, true));
    // Skala in VU relativ zum Ziel: 0 = Dreieck
    for (float vu : { 3.0f, 0.0f, -3.0f, -6.0f, -10.0f, -20.0f, -30.0f })
    {
        const float db = targetDb + vu;
        if (db > GainDSP::meterMaxDb || db < GainDSP::meterMinDb) continue;
        const float y = yForDb (db);
        g.setColour (Colours::white.withAlpha (0.18f));
        g.drawHorizontalLine ((int) y, pkBar.getRight() + 1.0f * s, pkBar.getRight() + 4.0f * s);
        g.setColour (textDim);
        g.drawText ((vu > 0.0f ? "+" : "") + String ((int) vu), Rectangle<float> (pkBar.getRight() + 5.0f * s, y - 6.0f * s, 22.0f * s, 12.0f * s),
                    Justification::centredLeft);
    }

    // 0 dBFS: feine Linie ueber den True-Peak-Balken
    g.setColour (Colours::white.withAlpha (0.55f));
    g.drawHorizontalLine ((int) yForDb (0.0f), pkBar.getX() - 1.0f * s, pkBar.getRight() + 1.0f * s);

    // Zielpegel: Linie ueber den RMS-Balken, Dreieck links (zum Ziehen)
    const float ty = yForDb (targetDb);
    g.setColour (text.withAlpha (0.8f));
    g.drawHorizontalLine ((int) ty, rmsBar.getX(), rmsBar.getRight());
    Path tri;
    const float tx = rmsBar.getX() - 4.0f * s;
    tri.addTriangle (tx - 11.0f * s, ty - 6.0f * s, tx - 11.0f * s, ty + 6.0f * s, tx, ty);
    g.setColour (levelColour (targetDb, targetDb).withAlpha (0.35f));
    g.fillPath (tri);
    g.setColour (text);
    g.strokePath (tri, PathStrokeType (1.6f));
}

void GainMeter::drag (const MouseEvent& e, int phase)
{
    if (! onTargetDrag) return;
    const float db = std::round (dbForY (e.position.y) * 2.0f) / 2.0f;
    onTargetDrag (phase, jlimit (-36.0f, -6.0f, db));
}

void GainMeter::mouseDown (const MouseEvent& e) { drag (e, 0); }
void GainMeter::mouseDrag (const MouseEvent& e) { drag (e, 1); }
void GainMeter::mouseUp (const MouseEvent& e)   { drag (e, 2); }
