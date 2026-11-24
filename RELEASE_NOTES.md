Free gain-staging utility for macOS (Apple Silicon), VST3, Audio Unit and CLAP. By Dani Koenig.

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
