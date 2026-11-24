#pragma once
#include <juce_graphics/juce_graphics.h>
#include <array>
#include <cmath>

// Themes: ein Theme ist ein Satz aus Farben und Stil-Optionen. Die Pegelfarben
// (blue = zu leise, green = Zielbereich, yellow = zu laut, red = Uebersteuerung) kommen aus dem Theme.
namespace GainColours
{
    struct Theme
    {
        const char* name = "JUCE";
        juce::Colour bgTop, bgBottom, card, cardBorder, inset, text, textDim, knobTrack, knobBody;
        juce::Colour blue, green, yellow, red;
        juce::Colour frame { juce::Colours::transparentBlack };   // fester Rahmen ums Fenster (JUCE: Orange wie bei den anderen Plugins)
        float radius = 6.0f;        // Eckenradius der Buttons und des Fensterrahmens
        float glowScale = 1.0f;     // Staerke des Leuchtens (Knob, Hintergrund, Rahmen)
        float titleSpacing = 12.0f; // Buchstabenabstand im Titel
        bool ticks = false;         // feine Skala um den Knob (Industrial)
        bool patterns = false;      // Warnzonen zusaetzlich als Muster (Farbenblind)
    };

    inline const Theme& themeFor (int index)
    {
        static const std::array<Theme, 5> all = []
        {
            std::array<Theme, 5> a;

            // 0: JUCE (das Original)
            {
                auto& t = a[0];
                t.name = "JUCE";
                t.bgTop = juce::Colour (0xff17181c);   t.bgBottom = juce::Colour (0xff0d0e11);
                t.card = juce::Colour (0xff1d1f24);    t.cardBorder = juce::Colour (0xff2b2e35);
                t.inset = juce::Colour (0xff121317);   t.text = juce::Colour (0xffeceef2);
                t.textDim = juce::Colour (0xff7d828c); t.knobTrack = juce::Colour (0xff2c2f36);
                t.knobBody = juce::Colour (0xff26282e);
                t.blue = juce::Colour (0xff4d8dff);    t.green = juce::Colour (0xff3ddc84);
                t.yellow = juce::Colour (0xffffc933);  t.red = juce::Colour (0xffff1f3f);
                t.frame = juce::Colour (0xffff7a2f);
            }
            // 1: Ableton-inspiriert: helles Grau, flach, wenig Leuchten
            {
                auto& t = a[1];
                t.name = "Ableton";
                t.bgTop = juce::Colour (0xffcbcbcb);   t.bgBottom = juce::Colour (0xffbdbdbd);
                t.card = juce::Colour (0xffd2d2d2);    t.cardBorder = juce::Colour (0xff8e8e8e);
                t.inset = juce::Colour (0xffa6a6a6);   t.text = juce::Colour (0xff141414);
                t.textDim = juce::Colour (0xff4f4f4f); t.knobTrack = juce::Colour (0xff9a9a9a);
                t.knobBody = juce::Colour (0xffdedede);
                t.blue = juce::Colour (0xff2f78d6);    t.green = juce::Colour (0xff17a455);
                t.yellow = juce::Colour (0xffe9a300);  t.red = juce::Colour (0xffd92626);
                t.radius = 2.0f; t.glowScale = 0.25f; t.titleSpacing = 10.0f;
            }
            // 2: Bitwig-inspiriert: dunkles Grau, flache Flaechen, kleine Radien
            {
                auto& t = a[2];
                t.name = "Bitwig";
                t.bgTop = juce::Colour (0xff2e2e31);   t.bgBottom = juce::Colour (0xff232325);
                t.card = juce::Colour (0xff3a3a3e);    t.cardBorder = juce::Colour (0xff505055);
                t.inset = juce::Colour (0xff1b1b1d);   t.text = juce::Colour (0xffe9e9ea);
                t.textDim = juce::Colour (0xff909096); t.knobTrack = juce::Colour (0xff48484d);
                t.knobBody = juce::Colour (0xff3c3c40);
                t.blue = juce::Colour (0xff3fa9f5);    t.green = juce::Colour (0xff8bd450);
                t.yellow = juce::Colour (0xffff9f1a);  t.red = juce::Colour (0xffff3b30);
                t.radius = 3.0f; t.glowScale = 0.45f; t.titleSpacing = 8.0f;
            }
            // 3: Dark Industrial: mattschwarz, Stahl, kalt, scharfe Ecken, Skala am Knob
            {
                auto& t = a[3];
                t.name = "Industrial";
                t.bgTop = juce::Colour (0xff0c0c0d);   t.bgBottom = juce::Colour (0xff040405);
                t.card = juce::Colour (0xff141415);    t.cardBorder = juce::Colour (0xff3d4045);
                t.inset = juce::Colour (0xff070708);   t.text = juce::Colour (0xfff2f4f6);
                t.textDim = juce::Colour (0xff8b9198); t.knobTrack = juce::Colour (0xff2b2e32);
                t.knobBody = juce::Colour (0xff1a1b1d);
                t.blue = juce::Colour (0xff6c7a89);    t.green = juce::Colour (0xffeef2f6);
                t.yellow = juce::Colour (0xffe8a33d);  t.red = juce::Colour (0xffff2a1a);
                t.radius = 1.5f; t.glowScale = 0.18f; t.titleSpacing = 18.0f; t.ticks = true;
            }
            // 4: Farbenblind: Blau - Weiss - Orange - Violett statt Rot/Gruen, plus Muster in der Warnzone
            {
                auto& t = a[4];
                t.name = "Colour-Blind";
                t.bgTop = juce::Colour (0xff17181c);   t.bgBottom = juce::Colour (0xff0d0e11);
                t.card = juce::Colour (0xff1d1f24);    t.cardBorder = juce::Colour (0xff3a3e46);
                t.inset = juce::Colour (0xff121317);   t.text = juce::Colour (0xfff4f4f4);
                t.textDim = juce::Colour (0xff8d929c); t.knobTrack = juce::Colour (0xff2c2f36);
                t.knobBody = juce::Colour (0xff26282e);
                t.blue = juce::Colour (0xff3a7bd5);    t.green = juce::Colour (0xfff2f2f2);
                t.yellow = juce::Colour (0xffe69f00);  t.red = juce::Colour (0xffcc79a7);
                t.patterns = true;
            }
            return a;
        }();
        return all[(size_t) juce::jlimit (0, (int) all.size() - 1, index)];
    }

    inline juce::StringArray themeNames()
    {
        juce::StringArray n;
        for (int i = 0; i < 5; ++i) n.add (themeFor (i).name);
        return n;
    }

    // Farbe fuer einen Pegel (dBFS) bei gegebenem Zielpegel:
    // Ziel = gruen, darunter immer blauer, darueber gelb, ab ca. -1 dBFS rot
    inline juce::Colour levelColour (const Theme& T, float db, float target)
    {
        auto smooth = [] (float a, float b, float v) { return juce::jlimit (0.0f, 1.0f, (v - a) / (b - a)); };
        juce::Colour c = T.green;
        if (db < target - 2.0f)
            c = T.green.interpolatedWith (T.blue, smooth (target - 2.0f, target - 14.0f, db));
        else if (db > target + 2.0f)
            c = T.green.interpolatedWith (T.yellow, smooth (target + 2.0f, target + 5.0f, db));
        if (db > -4.0f)
        {
            const juce::Colour base = (db > target + 2.0f) ? c : T.yellow;
            c = base.interpolatedWith (T.red, smooth (-4.0f, -1.0f, db));
        }
        return c;
    }
}

// Legt die Farben eines Themes als lokale Variablen an (gleiche Namen wie die Felder)
#define GAIN_LOCALS(T) \
    [[maybe_unused]] const juce::Colour bgTop = (T).bgTop, bgBottom = (T).bgBottom, card = (T).card, \
        cardBorder = (T).cardBorder, inset = (T).inset, text = (T).text, textDim = (T).textDim, \
        knobTrack = (T).knobTrack, knobBody = (T).knobBody, blue = (T).blue, green = (T).green, \
        yellow = (T).yellow, red = (T).red
