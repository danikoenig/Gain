# Gain

Kleines Gain-Staging-Utility fuer den Kanalanfang (macOS Apple Silicon, VST3, Audio Unit, CLAP). Von Dani Koenig.

- Grosser Gain-Knopf, +/-35 dB
- Meter: zwei Balken (links/rechts) mit RMS und True Peak uebereinander, Skala in dB relativ zum Ziel, Zielpegel per Dreieck, Farben blau / gruen / gelb / rot
- Calibration: 5 Sekunden zuhoeren, Gain auf den Zielpegel setzen (Messung nach den Filtern)
- Mono-Schalter ((L+R)/2), Low Cut (aus / 25 / 100 Hz), High Cut (aus / 18 kHz / 10 kHz), DC-Offset immer entfernt

Installation: siehe INSTALL.txt. Build: GitHub Actions (siehe .github/workflows/build.yml).

Test des DSP-Kerns: `g++ -std=c++17 -O2 -o enginetest tests/EngineTest.cpp Source/Engine.cpp && ./enginetest`
