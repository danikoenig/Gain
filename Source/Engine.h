#pragma once

#include <atomic>
#include <cmath>
#include <vector>
#include <algorithm>

// Reiner C++-DSP ohne JUCE-Abhaengigkeit (damit er separat getestet werden kann)
namespace GainDSP
{
    constexpr float maxGainDb   = 35.0f;
    constexpr float meterMinDb  = -48.0f;   // unteres Ende der Meter-Skala
    constexpr float meterMaxDb  = 3.0f;     // oberes Ende (True Peak kann ueber 0 liegen)
    constexpr float calSeconds  = 5.0f;
    constexpr float rmsOffsetDb = 3.0103f;  // AES-17: Sinus mit -18 dBFS Spitze = -18 Anzeige (VU 0 bei CAL -18)
    constexpr float vuSineScale = 1.1107f;  // Mittelwert gleichgerichtet -> Sinus-Effektivwert (pi / (2 sqrt 2))
    constexpr float calGateDb   = -55.0f;   // Bloecke leiser als das zaehlen bei der Messung nicht
    constexpr float lowCutHz[3]  = { 0.0f, 25.0f, 100.0f };
    constexpr float highCutHz[3] = { 0.0f, 18000.0f, 10000.0f };

    inline float toDb (float lin) { return 20.0f * std::log10 (std::max (lin, 1.0e-6f)); }
    inline float toLin (float db) { return std::pow (10.0f, db / 20.0f); }
}

struct GainParams
{
    float gainDb   = 0.0f;    // -35..+35
    int   lowCut   = 0;       // 0 = aus, 1 = 25 Hz, 2 = 100 Hz
    int   highCut  = 0;       // 0 = aus, 1 = 18 kHz, 2 = 10 kHz
    float targetDb = -18.0f;  // Zielpegel (VU 0, dBFS) fuer Calibration und Farben
    bool  mono     = false;   // Stereo -> Mono ((L+R)/2), nach den Filtern, vor Messung und Gain
};

class GainEngine
{
public:
    static constexpr int maxChannels = 2;

    void prepare (double sampleRate);
    void reset();
    void process (float* const* channels, int numChannels, int numSamples, const GainParams& p);

    // Calibration
    void startCalibration()  { calRequest = true; }
    void cancelCalibration() { calCancel = true; }
    bool isCalibrating() const { return calActive.load(); }
    float getCalProgress() const { return calProgress.load(); }
    // true, wenn ein neues Ergebnis da ist (gainDb), sonst false
    bool popCalibrationResult (float& gainDb)
    {
        if (! calDone.exchange (false)) return false;
        gainDb = calResultDb.load();
        return true;
    }
    bool popCalibrationFailed() { return calFailed.exchange (false); }

    // Anzeigen (dBFS, nach Filter und Gain)
    std::atomic<float> rmsDb { -100.0f }, peakDb { -100.0f };

private:
    struct Biquad
    {
        float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
        double z1 = 0, z2 = 0;
        void setHighpass (double fs, double fc);
        void setLowpass (double fs, double fc);
        inline float process (float x)
        {
            const double y = b0 * x + z1;
            z1 = b1 * x - a1 * y + z2;
            z2 = b2 * x - a2 * y;
            return (float) y;
        }
        void reset() { z1 = z2 = 0; }
    };

    struct Channel
    {
        double dcX1 = 0, dcY1 = 0;
        Biquad lc25, lc100, hc18, hc10;
        float hist[16] = {};
        int idx = 0;
        double vu = 0;   // VU-Ballistik: Mittelwert des gleichgerichteten Signals
    };

    // 4x True-Peak: gefensterte Sinc-Interpolation, 3 Zwischenwerte pro Sample
    float truePeakSample (Channel& c, float x);

    double fs = 48000.0;
    double dcR = 0.999;
    float kRamp = 0.01f, kGain = 0.001f;
    double kRms = 0.0001;
    float tpCoef[3][12] = {};

    Channel ch[maxChannels];
    float lcGain[3] = { 1, 0, 0 }, hcGain[3] = { 1, 0, 0 };
    float gainLin = 1.0f, monoMix = 0.0f;

    // Peak-Anzeige mit Halten und Abfall
    float peakLin = 0.0f;
    double peakHold = 0.0;

    // Calibration
    std::atomic<bool> calRequest { false }, calCancel { false }, calActive { false }, calDone { false }, calFailed { false };
    std::atomic<float> calProgress { 0.0f }, calResultDb { 0.0f };
    long calSamplesLeft = 0, calBlockLen = 1, calBlockPos = 0, calValid = 0;
    double calBlockSum = 0, calTotalPower = 0;
};
