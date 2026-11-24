Free gain-staging utility for macOS (Apple Silicon), VST3, Audio Unit and CLAP. By Dani Koenig.

### v1.5.1
- **Themes reduced to four and reworked:** JUCE (original, orange frame), Grey (all greys: darker display areas, lighter buttons), Dark (black, cold, steel frame, dial scale around the knob, darker blue and green, blood-red overload), Colour-Blind. Projects saved with v1.5.0 may show a different theme once; choose it again
- **Theme menu:** click (left or right) the header; the menu now opens directly under the title

### v1.5.0
- **Themes:** right-click the header (the GAIN title) to choose the look: JUCE (the original, now with the orange frame like the other plugins), Ableton (light grey, flat), Bitwig (dark grey, flat), Industrial (matte black and steel, sharp corners, dial scale around the knob), Colour-Blind (blue / white / orange / violet instead of red / green, with a striped pattern in the warning zone). The choice is saved with the project
- **Sweet spot:** after CALIBRATE, the exact gain it found is marked on the knob ring (small green tick). When you turn the knob across that value, the whole plugin flashes green briefly and stays a little greener while you sit on it, so you can easily get back without calibrating again. The value is saved with the project

### v1.4.0
- **Meter display:** right-click the bars to choose RMS (default), True Peak or RMS + True Peak. RMS alone is calmer and matches the target triangle; the white hold line still shows the true peak. The choice is saved with the project. Presets for the target stay on a right-click on the triangle
- RMS display is slower (time constant 300 ms), so it no longer jumps with every kick
- The plugin colour (knob, buttons, glow) follows a slow average (about 2 s) instead of the fast level, so it no longer flickers in time with the beat. Real peaks close to 0 dBFS still turn it red

### v1.3.0
- **Peak guard for CALIBRATE:** the gain is limited so the true peak stays below -3 dBFS, even if the RMS target is then not reached (button shows PEAK LIMIT for a moment). Helps with very peaky material such as drum loops
- **Target triangle:** while you drag it (or hover it) a label shows the exact value, e.g. "TARGET -18.0 dBFS RMS". Right-click the meter for presets (-18, -16, -14, -12, -9, -6 dBFS RMS)
- Same font and header style as the other plugins: big GAIN with a small "by Dani Koenig . version" next to it
- Knob: slimmer value arc, no longer cut off at the top

### v1.2.1
- **Fix:** CALIBRATE now measures the true RMS (power). Before, it used a rectified average, which reads far too low on peaky material such as drum loops, so the gain ended up several dB too high (peaks above 0 dBFS). The meter bars show the same RMS
- Wording: the meter shows RMS (about 300 ms) and the scale is in dB relative to the target

### v1.2.0
- Meter like Ableton: two bars (left / right), each with VU (darker) and true peak (lighter) on top, with a hold line

### v1.1.2
- Target is now a plain RMS level: -18 dBFS RMS (default) = a sine with -15 dBFS peak. Meter and CALIBRATE use this scale

### v1.1.1
- Cleaner header: big centred GAIN title; the fast numeric readouts and the target text are gone; footer shows "by Dani Koenig" and the version

### v1.1.0
- New MONO button (first in the row): sums left and right to (L+R)/2 before the meter, calibration and gain; buttons now in signal order: MONO, LOW CUT, HIGH CUT, CALIBRATE

### v1.0.0
- Big GAIN knob, +/-35 dB, 0 dB at the top
- Meter: VU (like VUMT) and true peak, scale in VU relative to the target; draggable target triangle (default -18 dBFS); blue / green / yellow / red zones, the whole plugin follows the colour
- CALIBRATE: listens for 5 s and sets GAIN to hit the target (measured after the filters), knob turns to the new value
- LOW CUT (off / 25 Hz / 100 Hz), HIGH CUT (off / 18 kHz / 10 kHz), DC offset always removed

**Download `Gain-Installer.pkg`** and double-click it. macOS blocks it the first time: open System Settings > Privacy & Security and click "Open Anyway". Full guide: see INSTALL.txt.

Requirements: Apple Silicon Mac (M1 or newer), macOS 11 or newer.
