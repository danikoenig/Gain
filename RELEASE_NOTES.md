Free gain-staging utility for macOS (Apple Silicon), VST3, Audio Unit and CLAP. By Dani Koenig.

### v1.1.0
- New MONO button (first in the row): sums left and right to (L+R)/2 before the meter, calibration and gain; buttons now in signal order: MONO, LOW CUT, HIGH CUT, CALIBRATE

### v1.0.0
- Big GAIN knob, +/-35 dB, 0 dB at the top
- Meter: VU (like VUMT) and true peak, scale in VU relative to the target; draggable target triangle (default -18 dBFS); blue / green / yellow / red zones, the whole plugin follows the colour
- CALIBRATE: listens for 5 s and sets GAIN to hit the target (measured after the filters), knob turns to the new value
- LOW CUT (off / 25 Hz / 100 Hz), HIGH CUT (off / 18 kHz / 10 kHz), DC offset always removed

**Download `Gain-Installer.pkg`** and double-click it. macOS blocks it the first time: open System Settings > Privacy & Security and click "Open Anyway". Full guide: see INSTALL.txt.

Requirements: Apple Silicon Mac (M1 or newer), macOS 11 or newer.
