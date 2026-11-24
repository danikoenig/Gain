# Gain

Kleines Gain-Staging-Utility fuer den Kanalanfang (macOS Apple Silicon, VST3, Audio Unit, CLAP). Von Dani Koenig.

- Grosser Gain-Knopf, +/-35 dB
- Meter: VU (wie VUMT) und True Peak, Skala in VU relativ zum Ziel, Zielpegel per Dreieck, Farben blau / gruen / gelb / rot
- Calibration: 5 Sekunden zuhoeren, Gain auf den Zielpegel setzen (Messung nach den Filtern)
- Low Cut (aus / 25 / 100 Hz), High Cut (aus / 18 kHz / 10 kHz), DC-Offset immer entfernt

Installation: siehe INSTALL.txt. Build: GitHub Actions (siehe .github/workflows/build.yml).

Test des DSP-Kerns: `g++ -std=c++17 -O2 -o enginetest tests/EngineTest.cpp Source/Engine.cpp && ./enginetest`
