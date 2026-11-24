#include "Engine.h"

using namespace GainDSP;

static constexpr double kPi = 3.14159265358979323846;

void GainEngine::Biquad::setHighpass (double fs, double fc)
{
    fc = std::min (fc, fs * 0.45);
    const double w0 = 2.0 * kPi * fc / fs, c = std::cos (w0), s = std::sin (w0);
    const double alpha = s / (2.0 * 0.70710678118654752);
    const double a0 = 1.0 + alpha;
    b0 = (float) (((1.0 + c) * 0.5) / a0);
    b1 = (float) (-(1.0 + c) / a0);
    b2 = b0;
    a1 = (float) (-2.0 * c / a0);
    a2 = (float) ((1.0 - alpha) / a0);
}

void GainEngine::Biquad::setLowpass (double fs, double fc)
{
    fc = std::min (fc, fs * 0.45);
    const double w0 = 2.0 * kPi * fc / fs, c = std::cos (w0), s = std::sin (w0);
    const double alpha = s / (2.0 * 0.70710678118654752);
    const double a0 = 1.0 + alpha;
    b0 = (float) (((1.0 - c) * 0.5) / a0);
    b1 = (float) ((1.0 - c) / a0);
    b2 = b0;
    a1 = (float) (-2.0 * c / a0);
    a2 = (float) ((1.0 - alpha) / a0);
}

void GainEngine::prepare (double sampleRate)
{
    fs = sampleRate;
    dcR = std::exp (-2.0 * kPi * 5.0 / fs);                 // DC-Filter: 1. Ordnung, 5 Hz
    kRamp = (float) (1.0 - std::exp (-1.0 / (0.004 * fs)));  // 4 ms: Filter ein/aus ohne Klick
    kGain = (float) (1.0 - std::exp (-1.0 / (0.020 * fs)));  // 20 ms Gain-Glaettung
    kRms = 1.0 - std::exp (-1.0 / (0.300 * fs));             // RMS-Anzeige: Zeitkonstante 300 ms (ruhig, ca. 900 ms bis fast voll)

    for (int ph = 0; ph < 3; ++ph)
    {
        const double f = 0.25 * (ph + 1);                    // 0.25, 0.5, 0.75
        for (int j = 0; j < 12; ++j)
        {
            const double x = (double) j - 6.0 + f;                // Abstand zum Zielpunkt zwischen h[6] und h[5]
            const double sinc = std::abs (x) < 1.0e-9 ? 1.0 : std::sin (kPi * x) / (kPi * x);
            const double win = std::abs (x) >= 6.0 ? 0.0 : 0.5 * (1.0 + std::cos (kPi * x / 6.0));
            tpCoef[ph][j] = (float) (sinc * win);
        }
    }

    for (auto& c : ch)
    {
        c.lc25.setHighpass (fs, 25.0);
        c.lc100.setHighpass (fs, 100.0);
        c.hc18.setLowpass (fs, 18000.0);
        c.hc10.setLowpass (fs, 10000.0);
    }
    reset();
}

void GainEngine::reset()
{
    for (auto& c : ch)
    {
        c.dcX1 = c.dcY1 = 0; c.ms = 0; c.idx = 0; c.idxPre = 0;
        std::fill (std::begin (c.histPre), std::end (c.histPre), 0.0f);
        c.lc25.reset(); c.lc100.reset(); c.hc18.reset(); c.hc10.reset();
        std::fill (std::begin (c.hist), std::end (c.hist), 0.0f);
    }
    for (int c = 0; c < maxChannels; ++c) { peakLin[c] = 0.0f; peakHold[c] = 0.0; }
    calActive = false; calProgress = 0.0f;
}

float GainEngine::truePeakSample (float* hist, int& idx, float x)
{
    idx = (idx + 1) & 15;
    hist[idx] = x;
    float pk = std::abs (hist[(idx - 6) & 15]);
    for (int ph = 0; ph < 3; ++ph)
    {
        float acc = 0.0f;
        for (int j = 0; j < 12; ++j)
            acc += hist[(idx - j) & 15] * tpCoef[ph][j];
        pk = std::max (pk, std::abs (acc));
    }
    return pk;
}

void GainEngine::process (float* const* data, int numCh, int n, const GainParams& p)
{
    numCh = std::min (numCh, (int) maxChannels);
    if (numCh <= 0 || n <= 0) return;

    // Calibration: Start / Abbruch
    if (calCancel.exchange (false)) { calActive = false; calProgress = 0.0f; }
    if (calRequest.exchange (false))
    {
        calActive = true;
        calSamplesLeft = (long) (calSeconds * fs);
        calBlockLen = (long) (0.1 * fs);
        calBlockPos = 0; calBlockSum = 0; calTotalPower = 0; calValid = 0; calPeakPre = 0.0f;
        for (auto& c : ch) { std::fill (std::begin (c.histPre), std::end (c.histPre), 0.0f); c.idxPre = 0; }
        calProgress = 0.0f;
    }
    const bool cal = calActive.load();

    float lcT[3] = { 0, 0, 0 }, hcT[3] = { 0, 0, 0 };
    lcT[std::max (0, std::min (2, p.lowCut))] = 1.0f;
    hcT[std::max (0, std::min (2, p.highCut))] = 1.0f;
    const float gainTarget = GainDSP::toLin (std::max (-maxGainDb, std::min (maxGainDb, p.gainDb)));

    float blockPeak[maxChannels] = {};
    const float monoTarget = (p.mono && numCh == 2) ? 1.0f : 0.0f;

    for (int i = 0; i < n; ++i)
    {
        for (int k = 0; k < 3; ++k)
        {
            lcGain[k] += (lcT[k] - lcGain[k]) * kRamp;
            hcGain[k] += (hcT[k] - hcGain[k]) * kRamp;
        }
        gainLin += (gainTarget - gainLin) * kGain;
        monoMix += (monoTarget - monoMix) * kRamp;

        float zc[maxChannels] = {};
        for (int c = 0; c < numCh; ++c)
        {
            auto& s = ch[c];
            float x = data[c][i];

            // DC-Offset immer entfernen
            const double dc = (double) x - s.dcX1 + dcR * s.dcY1;
            s.dcX1 = x; s.dcY1 = dc;
            x = (float) dc;

            // Low-Cut
            const float l25 = s.lc25.process (x), l100 = s.lc100.process (x);
            float y = lcGain[0] * x + lcGain[1] * l25 + lcGain[2] * l100;
            // High-Cut
            const float h18 = s.hc18.process (y), h10 = s.hc10.process (y);
            zc[c] = hcGain[0] * y + hcGain[1] * h18 + hcGain[2] * h10;
        }

        // Mono: (L+R)/2, weich ein- und ausgeblendet
        if (numCh == 2 && monoMix > 1.0e-4f)
        {
            const float mid = 0.5f * (zc[0] + zc[1]);
            zc[0] += (mid - zc[0]) * monoMix;
            zc[1] += (mid - zc[1]) * monoMix;
        }

        double preSq = 0.0;
        for (int c = 0; c < numCh; ++c)
        {
            auto& s = ch[c];
            const float z = zc[c];
            preSq += (double) z * z;          // Messpunkt fuer Calibration: nach Filtern und Mono, vor dem Gain (Leistung -> echter RMS)

            const float o = z * gainLin;
            data[c][i] = o;

            s.ms += ((double) o * o - s.ms) * kRms;
            blockPeak[c] = std::max (blockPeak[c], truePeakSample (s.hist, s.idx, o));
            if (cal) calPeakPre = std::max (calPeakPre, truePeakSample (s.histPre, s.idxPre, z));
        }

        if (cal)
        {
            calBlockSum += preSq / numCh;
            if (++calBlockPos >= calBlockLen)
            {
                const double pw = calBlockSum / (double) calBlockLen;
                if (10.0 * std::log10 (pw + 1.0e-20) + rmsOffsetDb > calGateDb) { calTotalPower += pw; ++calValid; }
                calBlockSum = 0; calBlockPos = 0;
            }
            --calSamplesLeft;
        }
    }

    // Anzeigen
    double ms = 0.0;
    for (int c = 0; c < numCh; ++c) ms += ch[c].ms;
    rmsDb = 10.0f * (float) std::log10 (ms / numCh + 1.0e-12) + rmsOffsetDb;

    // True Peak je Kanal mit Halten (1.5 s) und Abfall (ca. 20 dB/s)
    const double dt = (double) n / fs;
    float peakMax = 0.0f;
    for (int c = 0; c < numCh; ++c)
    {
        if (blockPeak[c] >= peakLin[c]) { peakLin[c] = blockPeak[c]; peakHold[c] = 1.5; }
        else if ((peakHold[c] -= dt) <= 0.0) peakLin[c] *= (float) std::pow (10.0, -dt);
        peakMax = std::max (peakMax, peakLin[c]);
        peakDbCh[c] = GainDSP::toDb (peakLin[c]);
        rmsDbCh[c] = 10.0f * (float) std::log10 (ch[c].ms + 1.0e-12) + rmsOffsetDb;
    }
    if (numCh == 1) { peakDbCh[1] = peakDbCh[0].load(); rmsDbCh[1] = rmsDbCh[0].load(); }   // Mono-Spur: beide Balken gleich
    peakDb = GainDSP::toDb (peakMax);

    // Calibration abschliessen
    if (cal)
    {
        calProgress = 1.0f - (float) std::max (0L, calSamplesLeft) / (calSeconds * (float) fs);
        if (calSamplesLeft <= 0)
        {
            calActive = false; calProgress = 0.0f;
            if (calValid >= 5)    // mindestens 0.5 s Signal
            {
                const float measured = 10.0f * (float) std::log10 (calTotalPower / (double) calValid + 1.0e-20) + rmsOffsetDb;
                float result = p.targetDb - measured;
                // Sicherung: auch bei spitzigem Material bleiben die True Peaks unter dem Limit
                const float peakLimitGain = calPeakCeilingDb - GainDSP::toDb (calPeakPre);
                if (peakLimitGain < result) { result = peakLimitGain; calLimited = true; }
                calResultDb = std::max (-maxGainDb, std::min (maxGainDb, result));
                calDone = true;
            }
            else
                calFailed = true;
        }
    }
}
