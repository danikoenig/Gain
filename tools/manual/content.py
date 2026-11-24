try:
    from lib import install_text
except ImportError:
    pass

THEME_IMAGES = [("themes/init.jpg", "Init"), ("themes/lightgray.jpg", "Light Gray"),
                ("themes/dark.jpg", "Dark"), ("themes/colorblind.jpg", "Colorblind")]

CONTENT = {
"title": "GAIN", "file": "Gain", "repo": "Gain",
"accent": "#3ddc84", "accent2": "#4d8dff",
"en": {
 "subtitle": "Gain staging utility",
 "kind": "Audio effect for macOS (Apple Silicon)  \xb7  VST3, Audio Unit, CLAP and Standalone",
 "by": "by Dani Koenig",
 "intro": ("Gain sits at the start of a channel and brings the signal to a good working level before it reaches your plugins. "
           "One big knob sets the gain (plus or minus 35 dB), a stereo meter shows the level, and CALIBRATE listens for 5 seconds and sets the gain for you so the average level lands on the target triangle. "
           "The whole plugin changes colour with the level: blue is too quiet, green is the sweet spot, yellow is too loud, red is overload."),
 "note": "Default target: -18 dBFS RMS (a sine with -15 dBFS peak). That is a good input level for classic compressor and tape plugins.",
 "blocks": [],
 "quick_title": "Quick start",
 "quick": [
   ("1. Insert", "Put Gain first on the channel, before your other plugins."),
   ("2. Play", "Play the loudest, most typical part of the track."),
   ("3. Calibrate", "Press <b>CALIBRATE</b>. Gain listens for 5 seconds, then the knob turns to the new value."),
   ("4. Check", "The meter should sit around the triangle and the plugin glows green."),
 ],
 "first": "Calibration never runs by itself. If you change the material or the filters, press CALIBRATE again.",
 "sections": [
  ("#4d8dff", "GAIN KNOB", [
    ("GAIN", "Plus or minus 35 dB. 0 dB is at the top. Double-click returns to 0 dB. The value is shown below the knob."),
    ("Colour", "Knob, buttons and glow follow the level: blue = too quiet, green = near the target, yellow = too loud, red = overload (above about -1 dBFS)."),
    ("Sweet spot", "After CALIBRATE a small green tick on the ring marks the gain it found. When you turn across that value the plugin flashes green and stays a bit greener while you are on it, so you can find your way back without calibrating again."),
  ]),
  ("#3ddc84", "BUTTONS (signal order)", [
    ("MONO", "Sums left and right to (L+R)/2 before the meter and the gain. Handy to check mono compatibility."),
    ("LOW CUT", "Off, 25 Hz or 100 Hz (12 dB per octave). Click to step through."),
    ("HIGH CUT", "Off, 18 kHz or 10 kHz (12 dB per octave). Click to step through."),
    ("CALIBRATE", "Listens for 5 seconds (LISTENING). Measures after the filters and mono, before the gain, then sets GAIN so the average level lands on the triangle. Quiet pauses below -55 dBFS are ignored."),
    ("NO SIGNAL", "Shown when there was no signal to measure. Nothing changes."),
    ("PEAK LIMIT", "The gain is limited so the true peak stays below -3 dBFS. The average level then stays under the target. It happens with very peaky material such as drum loops."),
  ]),
  ("#e0a800", "METER AND TARGET", [
    ("Bars", "Two bars, left and right. The meter shows the level after the gain, so it moves when you turn the knob."),
    ("Scale", "In dB relative to the target (0 = triangle). In True Peak mode the scale is absolute in dBFS. The thin line at the top is 0 dBFS."),
    ("Triangle", "Your target level. Drag it up or down (-36 to -6 dBFS RMS, in 0.5 dB steps). While you drag, a label shows the exact value."),
    ("Right-click triangle", "Presets: -18 (standard), -16, -14 (Studer), -12 (FET compressors), -9, -6 dBFS RMS."),
    ("Right-click bars", "Choose the display: <b>RMS</b> (default, calm), <b>True Peak</b> or <b>RMS + True Peak</b> (RMS darker, True Peak lighter on top)."),
    ("Hold line", "A thin white line holds the highest peak for 1.5 seconds."),
  ]),
 ],
 "extras": [
  {"title": "Themes",
   "text": "Click the header (the GAIN title) to choose the look. The choice is saved with the project.",
   "rows": [
     ("Init", "The original look, same as the other plugins, with the orange frame."),
     ("Light Gray", "All greys, display areas darker, buttons lighter."),
     ("Dark", "Black and cold like a pilot's watch: steel frame, dial scale around the knob, darker blue and green, blood-red overload."),
     ("Colorblind", "Blue, white, orange and violet instead of red and green, with a striped pattern in the warning zone of the meter."),
   ],
   "images": THEME_IMAGES},
 ],
 "recipes": [
   ("Drum bus", "Gain first, MONO off. Play the loop, press CALIBRATE. If PEAK LIMIT shows, the drums are very peaky: that is fine."),
   ("Compressor input", "Set the triangle to -18 (or -14 for a Studer feel), calibrate, then add the compressor and set its threshold."),
   ("Mono check", "Switch MONO on and watch the level. A big drop means phase problems."),
   ("Clean low end", "LOW CUT 25 Hz removes sub rumble that eats headroom, then calibrate again."),
 ],
 "tips": [
   "Gain measures RMS (the average level), not the loudest peak. Use True Peak to check headroom.",
   "A DC filter (5 Hz) is always on. It cannot be switched off.",
   "All settings can be automated. The sweet spot and the theme are saved with the project.",
   "You can resize the window. It keeps its shape.",
 ],
 "install": install_text("en", "Gain", "Gain", "Gain"),
},
"de": {
 "subtitle": "Gain-Staging-Utility",
 "kind": "Audio-Effekt für macOS (Apple Silicon)  \xb7  VST3, Audio Unit, CLAP und Standalone",
 "by": "von Dani Koenig",
 "intro": ("Gain steht am Anfang eines Kanals und bringt das Signal auf einen guten Arbeitspegel, bevor es in deine Plugins läuft. "
           "Ein grosser Knopf stellt den Gain ein (plus oder minus 35 dB), ein Stereo-Meter zeigt den Pegel, und CALIBRATE hört 5 Sekunden zu und stellt den Gain so ein, dass der durchschnittliche Pegel auf dem Ziel-Dreieck landet. "
           "Das ganze Plugin ändert seine Farbe mit dem Pegel: Blau ist zu leise, Grün ist der Sweet Spot, Gelb ist zu laut, Rot ist Übersteuerung."),
 "note": "Standard-Ziel: -18 dBFS RMS (ein Sinus mit -15 dBFS Spitze). Das ist ein guter Eingangspegel für klassische Kompressor- und Tape-Plugins.",
 "blocks": [],
 "quick_title": "Schnellstart",
 "quick": [
   ("1. Einfügen", "Gain als erstes Plugin in den Kanal legen, vor die anderen Plugins."),
   ("2. Abspielen", "Den lautesten, typischen Teil des Tracks abspielen."),
   ("3. Kalibrieren", "<b>CALIBRATE</b> drücken. Gain hört 5 Sekunden zu, dann dreht sich der Knopf auf den neuen Wert."),
   ("4. Prüfen", "Das Meter sollte um das Dreieck liegen und das Plugin leuchtet grün."),
 ],
 "first": "Die Kalibrierung läuft nie von selbst. Wenn sich Material oder Filter ändern, CALIBRATE noch einmal drücken.",
 "sections": [
  ("#4d8dff", "GAIN-KNOPF", [
    ("GAIN", "Plus oder minus 35 dB. 0 dB ist oben. Doppelklick setzt auf 0 dB zurück. Der Wert steht unter dem Knopf."),
    ("Farbe", "Knopf, Buttons und Leuchten folgen dem Pegel: Blau = zu leise, Grün = nahe am Ziel, Gelb = zu laut, Rot = Übersteuerung (über etwa -1 dBFS)."),
    ("Sweet Spot", "Nach CALIBRATE markiert ein kleiner grüner Strich auf dem Ring den gefundenen Gain. Drehst du über diesen Wert, blitzt das Plugin grün auf und bleibt etwas grüner, solange du darauf stehst. So findest du zurück, ohne neu zu kalibrieren."),
  ]),
  ("#3ddc84", "BUTTONS (in Signalreihenfolge)", [
    ("MONO", "Summiert links und rechts zu (L+R)/2 vor Meter und Gain. Praktisch, um die Mono-Kompatibilität zu prüfen."),
    ("LOW CUT", "Aus, 25 Hz oder 100 Hz (12 dB pro Oktave). Klick schaltet weiter."),
    ("HIGH CUT", "Aus, 18 kHz oder 10 kHz (12 dB pro Oktave). Klick schaltet weiter."),
    ("CALIBRATE", "Hört 5 Sekunden zu (LISTENING). Misst nach den Filtern und Mono, vor dem Gain, und stellt dann GAIN so ein, dass der durchschnittliche Pegel auf dem Dreieck landet. Leise Pausen unter -55 dBFS werden ignoriert."),
    ("NO SIGNAL", "Erscheint, wenn kein Signal zum Messen da war. Es ändert sich nichts."),
    ("PEAK LIMIT", "Der Gain wird begrenzt, damit der True Peak unter -3 dBFS bleibt. Der durchschnittliche Pegel bleibt dann unter dem Ziel. Das passiert bei sehr spitzem Material wie Drum-Loops."),
  ]),
  ("#e0a800", "METER UND ZIEL", [
    ("Balken", "Zwei Balken, links und rechts. Das Meter zeigt den Pegel nach dem Gain und bewegt sich also, wenn du am Knopf drehst."),
    ("Skala", "In dB relativ zum Ziel (0 = Dreieck). Im True-Peak-Modus ist die Skala absolut in dBFS. Die feine Linie oben ist 0 dBFS."),
    ("Dreieck", "Dein Zielpegel. Nach oben oder unten ziehen (-36 bis -6 dBFS RMS, in 0,5-dB-Schritten). Beim Ziehen zeigt ein Feld den genauen Wert."),
    ("Rechtsklick Dreieck", "Richtwerte: -18 (Standard), -16, -14 (Studer), -12 (FET-Kompressoren), -9, -6 dBFS RMS."),
    ("Rechtsklick Balken", "Anzeige wählen: <b>RMS</b> (Standard, ruhig), <b>True Peak</b> oder <b>RMS + True Peak</b> (RMS dunkler, True Peak heller darüber)."),
    ("Hold-Linie", "Eine feine weisse Linie hält die höchste Spitze 1,5 Sekunden."),
  ]),
 ],
 "extras": [
  {"title": "Themes",
   "text": "Klick auf die Kopfzeile (den Titel GAIN), um das Aussehen zu wählen. Die Wahl wird mit dem Projekt gespeichert.",
   "rows": [
     ("Init", "Das Original, gleich wie bei den anderen Plugins, mit dem orangen Rahmen."),
     ("Light Gray", "Alles in Grau, Anzeigeflächen dunkler, Buttons heller."),
     ("Dark", "Schwarz und kalt wie eine Fliegeruhr: Stahlrahmen, Skala um den Knopf, dunkleres Blau und Grün, Übersteuerung in Blutrot."),
     ("Colorblind", "Blau, Weiss, Orange und Violett statt Rot und Grün, dazu ein Streifenmuster in der Warnzone des Meters."),
   ],
   "images": THEME_IMAGES},
 ],
 "recipes": [
   ("Drum-Bus", "Gain zuerst, MONO aus. Loop abspielen, CALIBRATE drücken. Zeigt es PEAK LIMIT, sind die Drums sehr spitz: das ist in Ordnung."),
   ("Kompressor-Eingang", "Dreieck auf -18 (oder -14 für Studer-Gefühl), kalibrieren, dann den Kompressor einfügen und den Threshold einstellen."),
   ("Mono-Check", "MONO einschalten und den Pegel beobachten. Ein grosser Abfall deutet auf Phasenprobleme."),
   ("Sauberer Bass", "LOW CUT 25 Hz entfernt Sub-Rumpeln, das Headroom frisst, danach noch einmal kalibrieren."),
 ],
 "tips": [
   "Gain misst RMS (den durchschnittlichen Pegel), nicht die lauteste Spitze. Mit True Peak prüfst du den Headroom.",
   "Ein DC-Filter (5 Hz) ist immer an. Er lässt sich nicht ausschalten.",
   "Alle Einstellungen lassen sich automatisieren. Sweet Spot und Theme werden mit dem Projekt gespeichert.",
   "Das Fenster lässt sich in der Grösse ändern und behält seine Form.",
 ],
 "install": install_text("de", "Gain", "Gain", "Gain"),
},
}
