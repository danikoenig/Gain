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
        const char* name = "Init";
        const char* fontName = "";   // leer = JUCE-Standardschrift
        juce::Colour bgTop, bgBottom, card, cardBorder, inset, text, textDim, knobTrack, knobBody;
        juce::Colour blue, green, yellow, red;
        juce::Colour frame { juce::Colours::transparentBlack };   // fester Rahmen ums Fenster (transparent = keiner)
        juce::Colour btn, btnHover, btnText, btnTextDim;          // Buttons (leer = aus inset/cardBorder/text abgeleitet)
        float radius = 6.0f;        // Eckenradius der Buttons und des Fensterrahmens
        float glowScale = 1.0f;     // Staerke des Leuchtens (Knob, Hintergrund, Rahmen)
        float titleSpacing = 12.0f; // Buchstabenabstand im Titel
        bool ticks = false;         // feine Skala um den Knob (Dark)
        bool patterns = false;      // Warnzonen zusaetzlich als Muster (Colour-Blind)
    };

    inline const Theme& themeFor (int index)
    {
        static const std::array<Theme, 4> all = []
        {
            std::array<Theme, 4> a;
            auto C = [] (juce::uint32 v) { return juce::Colour (v); };

            // 0: Init (das Original, mit orangem Rahmen wie die anderen Plugins)
            {
                auto& t = a[0];
                t.name = "Init";
                t.bgTop = C (0xff17181c);   t.bgBottom = C (0xff0d0e11);
                t.card = C (0xff1d1f24);    t.cardBorder = C (0xff2b2e35);
                t.inset = C (0xff121317);   t.text = C (0xffeceef2);
                t.textDim = C (0xff7d828c); t.knobTrack = C (0xff2c2f36);
                t.knobBody = C (0xff26282e);
                t.blue = C (0xff4d8dff);    t.green = C (0xff3ddc84);
                t.yellow = C (0xffffc933);  t.red = C (0xffff1f3f);
                t.frame = C (0xffff7a2f);
            }
            // 1: Light Gray (an Ableton / Bitwig angelehnt): alles Grau, Anzeigeflaechen dunkler, Buttons heller
            {
                auto& t = a[1];
                t.name = "Light Gray";
                t.fontName = "Avenir Next";
                t.bgTop = C (0xff535353);   t.bgBottom = C (0xff494949);
                t.card = C (0xff5c5c5c);    t.cardBorder = C (0xff2a2a2a);
                t.inset = C (0xff1f1f1f);   t.text = C (0xffeeeeee);
                t.textDim = C (0xffb4b4b4); t.knobTrack = C (0xff3b3b3b);
                t.knobBody = C (0xff676767);
                t.blue = C (0xff55a9e6);    t.green = C (0xff8bd35c);
                t.yellow = C (0xffe6b84a);  t.red = C (0xffe0463c);
                t.frame = C (0xff151515);
                t.btn = C (0xff717171);     t.btnHover = C (0xff808080);
                t.btnText = C (0xff101010); t.btnTextDim = C (0xff333333);
                t.radius = 2.0f; t.glowScale = 0.3f; t.titleSpacing = 10.0f;
            }
            // 2: Dark: schwarz, kalt wie eine Fliegeruhr mit Stahl, dunkles Blau / Gruen, Uebersteuerung blutrot
            {
                auto& t = a[2];
                t.name = "Dark";
                t.fontName = "Helvetica Neue";
                t.bgTop = C (0xff0a0a0b);   t.bgBottom = C (0xff030304);
                t.card = C (0xff111214);    t.cardBorder = C (0xff373b40);
                t.inset = C (0xff050506);   t.text = C (0xffeef1f4);
                t.textDim = C (0xff868d95); t.knobTrack = C (0xff24272b);
                t.knobBody = C (0xff141518);
                t.blue = C (0xff2c5fa8);    t.green = C (0xff2fa36b);
                t.yellow = C (0xffcf7a1a);  t.red = C (0xffb0101c);
                t.frame = C (0xff6d747c);
                t.btnHover = C (0xff1a1c1f);
                t.radius = 1.5f; t.glowScale = 0.35f; t.titleSpacing = 18.0f; t.ticks = true;
            }
            // 3: Colour-Blind: Blau - Weiss - Orange - Violett statt Rot/Gruen, plus Muster in der Warnzone
            {
                auto& t = a[3];
                t.name = "Colorblind";
                t.bgTop = C (0xff17181c);   t.bgBottom = C (0xff0d0e11);
                t.card = C (0xff1d1f24);    t.cardBorder = C (0xff3a3e46);
                t.inset = C (0xff121317);   t.text = C (0xfff4f4f4);
                t.textDim = C (0xff8d929c); t.knobTrack = C (0xff2c2f36);
                t.knobBody = C (0xff26282e);
                t.blue = C (0xff3a7bd5);    t.green = C (0xfff2f2f2);
                t.yellow = C (0xffe69f00);  t.red = C (0xffcc79a7);
                t.patterns = true;
            }
            // Buttons: ohne eigene Angabe wie bisher (dunkle Flaeche, Rand beim Ueberfahren)
            for (auto& t : a)
            {
                if (t.btn.getAlpha() == 0)        t.btn = t.inset;
                if (t.btnHover.getAlpha() == 0)   t.btnHover = t.cardBorder;
                if (t.btnText.getAlpha() == 0)    t.btnText = t.text;
                if (t.btnTextDim.getAlpha() == 0) t.btnTextDim = t.textDim;
            }
            return a;
        }();
        return all[(size_t) juce::jlimit (0, (int) all.size() - 1, index)];
    }

    inline juce::StringArray themeNames()
    {
        juce::StringArray n;
        for (int i = 0; i < 4; ++i) n.add (themeFor (i).name);
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
        yellow = (T).yellow, red = (T).red, btn = (T).btn, btnHover = (T).btnHover, btnText = (T).btnText, btnTextDim = (T).btnTextDim
