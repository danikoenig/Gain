#pragma once
#include <juce_graphics/juce_graphics.h>
#include <cmath>

namespace GainColours
{
    const juce::Colour bgTop      { 0xff17181c };
    const juce::Colour bgBottom   { 0xff0d0e11 };
    const juce::Colour card       { 0xff1d1f24 };
    const juce::Colour cardBorder { 0xff2b2e35 };
    const juce::Colour inset      { 0xff121317 };
    const juce::Colour text       { 0xffeceef2 };
    const juce::Colour textDim    { 0xff7d828c };
    const juce::Colour knobTrack  { 0xff2c2f36 };
    const juce::Colour knobBody   { 0xff26282e };

    const juce::Colour blue   { 0xff4d8dff };   // zu leise
    const juce::Colour green  { 0xff3ddc84 };   // Zielbereich
    const juce::Colour yellow { 0xffffc933 };   // zu laut
    const juce::Colour red    { 0xffff1f3f };   // Uebersteuerung

    // Farbe fuer einen Pegel (dBFS) bei gegebenem Zielpegel:
    // Ziel = gruen, darunter immer blauer, darueber gelb, ab ca. -1 dBFS rot
    inline juce::Colour levelColour (float db, float target)
    {
        auto smooth = [] (float a, float b, float v) { return juce::jlimit (0.0f, 1.0f, (v - a) / (b - a)); };
        juce::Colour c = green;
        if (db < target - 2.0f)
            c = green.interpolatedWith (blue, smooth (target - 2.0f, target - 14.0f, db));
        else if (db > target + 2.0f)
            c = green.interpolatedWith (yellow, smooth (target + 2.0f, target + 5.0f, db));
        if (db > -4.0f)
        {
            const juce::Colour base = (db > target + 2.0f) ? c : yellow;
            c = base.interpolatedWith (red, smooth (-4.0f, -1.0f, db));
        }
        return c;
    }
}
