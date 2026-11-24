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
    kRms = 1.0 - std::exp (-1.0 / (0.065 * fs));             // VU-Ballistik: 99 % nach 300 ms

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
        c.dcX1 = c.dcY1 = 0; c.vu = 0; c.idx = 0;
        c.lc25.reset(); c.lc100.reset(); c.hc18.reset(); c.hc10.reset();
        std::fill (std::begin (c.hist), std::end (c.hist), 0.0f);
    }
    peakLin = 0; peakHold = 0;
    calActive = false; calProgress = 0.0f;
}

float GainEngine::truePeakSample (Channel& c, float x)
{
    c.idx = (c.idx + 1) & 15;
    c.hist[c.idx] = x;
    float pk = std::abs (c.hist[(c.idx - 6) & 15]);
    for (int ph = 0; ph < 3; ++ph)
    {
        float acc = 0.0f;
        for (int j = 0; j < 12; ++j)
            acc += c.hist[(c.idx - j) & 15] * tpCoef[ph][j];
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
        calBlockPos = 0; calBlockSum = 0; calTotalPower = 0; calValid = 0;
        calProgress = 0.0f;
    }
    const bool cal = calActive.load();

    float lcT[3] = { 0, 0, 0 }, hcT[3] = { 0, 0, 0 };
    lcT[std::max (0, std::min (2, p.lowCut))] = 1.0f;
    hcT[std::max (0, std::min (2, p.highCut))] = 1.0f;
    const float gainTarget = GainDSP::toLin (std::max (-maxGainDb, std::min (maxGainDb, p.gainDb)));

    float blockPeak = 0.0f;

    for (int i = 0; i < n; ++i)
    {
        for (int k = 0; k < 3; ++k)
        {
            lcGain[k] += (lcT[k] - lcGain[k]) * kRamp;
            hcGain[k] += (hcT[k] - hcGain[k]) * kRamp;
        }
        gainLin += (gainTarget - gainLin) * kGain;

        double preSq = 0.0;
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
            const float z = hcGain[0] * y + hcGain[1] * h18 + hcGain[2] * h10;

            preSq += std::abs ((double) z);   // Messpunkt fuer Calibration: nach den Filtern, vor dem Gain

            const float o = z * gainLin;
            data[c][i] = o;

            s.vu += ((double) std::abs (o) - s.vu) * kRms;
            blockPeak = std::max (blockPeak, truePeakSample (s, o));
        }

        if (cal)
        {
            calBlockSum += preSq / numCh;
            if (++calBlockPos >= calBlockLen)
            {
                const double avg = calBlockSum / (double) calBlockLen * vuSineScale;
                if (20.0 * std::log10 (avg + 1.0e-10) + rmsOffsetDb > calGateDb) { calTotalPower += avg; ++calValid; }
                calBlockSum = 0; calBlockPos = 0;
            }
            --calSamplesLeft;
        }
    }

    // Anzeigen
    double vu = 0.0;
    for (int c = 0; c < numCh; ++c) vu += ch[c].vu;
    rmsDb = 20.0f * (float) std::log10 (vu / numCh * vuSineScale + 1.0e-6) + rmsOffsetDb;

    const double dt = (double) n / fs;
    if (blockPeak >= peakLin) { peakLin = blockPeak; peakHold = 1.5; }
    else if ((peakHold -= dt) <= 0.0) peakLin *= (float) std::pow (10.0, -dt);   // ca. 20 dB/s Abfall
    peakDb = GainDSP::toDb (peakLin);

    // Calibration abschliessen
    if (cal)
    {
        calProgress = 1.0f - (float) std::max (0L, calSamplesLeft) / (calSeconds * (float) fs);
        if (calSamplesLeft <= 0)
        {
            calActive = false; calProgress = 0.0f;
            if (calValid >= 5)    // mindestens 0.5 s Signal
            {
                const float measured = 20.0f * (float) std::log10 (calTotalPower / (double) calValid + 1.0e-10) + rmsOffsetDb;
                calResultDb = std::max (-maxGainDb, std::min (maxGainDb, p.targetDb - measured));
                calDone = true;
            }
            else
                calFailed = true;
        }
    }
}
