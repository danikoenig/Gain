#include "../Source/Engine.h"
#include <cstdio>
#include <vector>
#include <cmath>

static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { std::printf ("FAIL: %s\n", msg); ++fails; } else std::printf ("ok:   %s\n", msg); } while (0)

static std::vector<float> sine (double fs, double f, double amp, int n, double dc = 0.0)
{
    std::vector<float> v (n);
    for (int i = 0; i < n; ++i) v[i] = (float) (dc + amp * std::sin (2.0 * M_PI * f * i / fs));
    return v;
}
static double rmsDb (const std::vector<float>& v, int skip)
{
    double s = 0; for (size_t i = skip; i < v.size(); ++i) s += (double) v[i] * v[i];
    return 10.0 * std::log10 (s / (v.size() - skip) + 1e-20);
}
static std::vector<float> run (GainEngine& e, std::vector<float> in, const GainParams& p)
{
    float* ch[1] = { in.data() };
    const int blk = 512;
    for (size_t i = 0; i < in.size(); i += blk)
    {
        float* c[1] = { in.data() + i };
        e.process (c, 1, (int) std::min<size_t> (blk, in.size() - i), p);
    }
    (void) ch;
    return in;
}

int main()
{
    const double fs = 48000.0;
    GainParams p;

    { // DC wird entfernt
        GainEngine e; e.prepare (fs);
        auto out = run (e, sine (fs, 1000, 0.1, 96000, 0.3), p);
        double mean = 0; for (size_t i = 48000; i < out.size(); ++i) mean += out[i]; mean /= (out.size() - 48000);
        CHECK (std::abs (mean) < 1e-3, "DC offset removed");
    }
    { // Gain
        GainEngine e; e.prepare (fs); p.gainDb = 12.0f;
        auto in = sine (fs, 1000, 0.1, 96000);
        auto out = run (e, in, p);
        CHECK (std::abs (rmsDb (out, 48000) - rmsDb (in, 48000) - 12.0) < 0.1, "gain +12 dB");
        p.gainDb = 0.0f;
    }
    { // Low-Cut 25 Hz: 25 Hz um ca. -3 dB, 1 kHz unveraendert
        GainEngine e; e.prepare (fs); p.lowCut = 1;
        auto a = run (e, sine (fs, 25, 0.5, 96000), p);
        GainEngine e2; e2.prepare (fs);
        auto b = run (e2, sine (fs, 1000, 0.5, 96000), p);
        auto in25 = sine (fs, 25, 0.5, 96000), in1k = sine (fs, 1000, 0.5, 96000);
        const double d25 = rmsDb (a, 48000) - rmsDb (in25, 48000), d1k = rmsDb (b, 48000) - rmsDb (in1k, 48000);
        std::printf ("      25Hz: %.2f dB, 1k: %.2f dB\n", d25, d1k);
        CHECK (std::abs (d25 + 3.0) < 0.6 && std::abs (d1k) < 0.1, "low cut 25 Hz");
        p.lowCut = 0;
    }
    { // Low-Cut 100 Hz
        GainEngine e; e.prepare (fs); p.lowCut = 2;
        auto in = sine (fs, 100, 0.5, 96000);
        auto a = run (e, in, p);
        const double d = rmsDb (a, 48000) - rmsDb (in, 48000);
        std::printf ("      100Hz: %.2f dB\n", d);
        CHECK (std::abs (d + 3.0) < 0.6, "low cut 100 Hz");
        p.lowCut = 0;
    }
    { // High-Cut 10 kHz und 18 kHz
        for (int s = 1; s <= 2; ++s)
        {
            GainEngine e; e.prepare (fs); p.highCut = s;
            const double fc = s == 1 ? 18000.0 : 10000.0;
            auto in = sine (fs, fc, 0.5, 96000);
            auto a = run (e, in, p);
            const double d = rmsDb (a, 48000) - rmsDb (in, 48000);
            std::printf ("      high cut %g: %.2f dB\n", fc, d);
            CHECK (std::abs (d + 3.0) < 1.5, s == 1 ? "high cut 18k" : "high cut 10k");
        }
        p.highCut = 0;
    }
    { // True Peak: Sinus mit fs/4 bei Phase 45 Grad: Sample-Peak 0.707, True Peak 1.0
        GainEngine e; e.prepare (fs);
        std::vector<float> v (48000);
        for (size_t i = 0; i < v.size(); ++i) v[i] = (float) std::sin (2.0 * M_PI * (fs / 4.0) * i / fs + M_PI / 4.0);
        run (e, v, p);
        const float pk = e.peakDb.load();
        std::printf ("      true peak: %.2f dBFS (Sample-Peak -3.01)\n", pk);
        CHECK (pk > -0.6 && pk < 0.3, "true peak sees inter-sample peak");
    }
    { // RMS-Anzeige: normaler RMS in dBFS, Sinus mit -15 dBFS Spitze => -18
        GainEngine e; e.prepare (fs);
        run (e, sine (fs, 1000, std::pow (10.0, -15.0 / 20.0), 96000), p);
        std::printf ("      rms: %.2f\n", e.rmsDb.load());
        CHECK (std::abs (e.rmsDb.load() + 18.0) < 0.3, "rms meter");
    }
    { // RMS-Ballistik: nach 300 ms fast voll (Abweichung < 0.4 dB), nach 100 ms deutlich darunter
        auto s = sine (fs, 1000, std::pow (10.0, -15.0 / 20.0), 48000);
        GainEngine e2; e2.prepare (fs);
        float* b[1] = { s.data() + 20000 }; e2.process (b, 1, 14400, p);   // 300 ms
        GainEngine e3; e3.prepare (fs);
        float* c[1] = { s.data() + 30000 }; e3.process (c, 1, 4800, p);    // 100 ms
        std::printf ("      VU after 300 ms: %.2f, after 100 ms: %.2f\n", e2.rmsDb.load(), e3.rmsDb.load());
        CHECK (e2.rmsDb.load() > -18.4 && e3.rmsDb.load() < -19.5, "RMS meter ballistics");
    }
    { // Calibration: Sinus mit -3 dBFS Spitze (RMS -6), Ziel -18 => Gain -12 dB
        GainEngine e; e.prepare (fs); p.targetDb = -18.0f; p.gainDb = 7.0f;   // Gain darf die Messung nicht beeinflussen
        e.startCalibration();
        auto in = sine (fs, 440, std::pow (10.0, -3.0 / 20.0), 48000 * 7);   // Spitze -3 dBFS
        run (e, in, p);
        float g = 0; const bool done = e.popCalibrationResult (g);
        std::printf ("      calibration gain: %.2f\n", g);
        CHECK (done && std::abs (g + 12.0f) < 0.3f, "calibration result -12 dB");
    }
    { // Calibration bei Stille: Fehler, kein Ergebnis
        GainEngine e; e.prepare (fs);
        e.startCalibration();
        run (e, std::vector<float> (48000 * 6, 0.0f), p);
        float g; const bool done = e.popCalibrationResult (g);
        CHECK (! done && e.popCalibrationFailed(), "calibration fails on silence");
    }
    { // Calibration clamp: sehr leises Signal => max +35 dB
        GainEngine e; e.prepare (fs); p.targetDb = -6.0f;
        e.startCalibration();
        run (e, sine (fs, 440, std::pow (10.0, -50.0 / 20.0), 48000 * 6), p);
        float g = 0; e.popCalibrationResult (g);
        CHECK (std::abs (g - 35.0f) < 0.01f, "calibration clamps to +35 dB");
    }
    { // Mono: gegenphasig -> Stille, nur links -> beide Seiten halbe Amplitude
        GainEngine e; e.prepare (fs); GainParams m; m.mono = true;
        std::vector<float> L = sine (fs, 1000, 0.5, 48000), R = L;
        for (auto& v : R) v = -v;
        for (size_t i = 0; i < L.size(); i += 512)
        {
            float* c[2] = { L.data() + i, R.data() + i };
            e.process (c, 2, (int) std::min<size_t> (512, L.size() - i), m);
        }
        CHECK (rmsDb (L, 24000) < -90.0 && rmsDb (R, 24000) < -90.0, "mono cancels out-of-phase signal");
        GainEngine e2; e2.prepare (fs);
        std::vector<float> A = sine (fs, 1000, 0.5, 48000), B (48000, 0.0f);
        const double inDb = rmsDb (A, 24000);
        for (size_t i = 0; i < A.size(); i += 512)
        {
            float* c[2] = { A.data() + i, B.data() + i };
            e2.process (c, 2, (int) std::min<size_t> (512, A.size() - i), m);
        }
        std::printf ("      mono: L %.2f dB, R %.2f dB (in %.2f)\n", rmsDb (A, 24000), rmsDb (B, 24000), inDb);
        CHECK (std::abs (rmsDb (A, 24000) - (inDb - 6.02)) < 0.3 && std::abs (rmsDb (A, 24000) - rmsDb (B, 24000)) < 0.05, "mono sums to (L+R)/2");
    }
    { // Drum-artiges Signal (Kick alle 500 ms): Calibration muss den ECHTEN RMS auf das Ziel bringen
        GainEngine e; e.prepare (fs); GainParams q; q.targetDb = -18.0f;
        std::vector<float> x (48000 * 6, 0.0f);
        for (int k = 0; k < 12; ++k)
            for (int i = 0; i < 12000; ++i)
            {
                const size_t n = (size_t) k * 24000 + i; if (n >= x.size()) break;
                x[n] = (float) (0.9 * std::exp (-i / 2500.0) * std::sin (2.0 * M_PI * 60.0 * i / fs));
            }
        for (size_t i = 0; i < x.size(); ++i) x[i] += 0.005f * (float) std::sin (2.0 * M_PI * 3137.0 * i / fs);   // leiser Boden (-46 dBFS), keine digitale Stille
        auto in = x;
        e.startCalibration();
        run (e, x, q);
        float g = 0; e.popCalibrationResult (g);
        double sq = 0; for (size_t i = 0; i < 48000 * 5; ++i) sq += (double) in[i] * in[i];
        const double rmsIn = 10.0 * std::log10 (sq / (48000.0 * 5.0));
        std::printf ("      kick loop: true RMS %.2f dBFS, calibration gain %.2f dB (expected %.2f)\n", rmsIn, g, -18.0 - rmsIn);
        CHECK (std::abs (g - (-18.0f - (float) rmsIn)) < 0.5f, "calibration lands the TRUE RMS on target (peaky material)");
    }
    { // Peak-Sicherung: Kick-Loop auf -10 dBFS RMS kalibrieren wuerde die Spitzen weit ueber -3 dBFS bringen
        GainEngine e; e.prepare (fs); GainParams q; q.targetDb = -10.0f;
        std::vector<float> x (48000 * 6, 0.0f);
        for (int k = 0; k < 12; ++k)
            for (int i = 0; i < 12000; ++i)
            {
                const size_t n = (size_t) k * 24000 + i; if (n >= x.size()) break;
                x[n] = (float) (0.9 * std::exp (-i / 2500.0) * std::sin (2.0 * M_PI * 60.0 * i / fs));
            }
        for (size_t i = 0; i < x.size(); ++i) x[i] += 0.005f * (float) std::sin (2.0 * M_PI * 3137.0 * i / fs);
        auto in = x;
        e.startCalibration();
        run (e, x, q);
        float g = 0; const bool done = e.popCalibrationResult (g);
        float pk = 0; for (size_t i = 0; i < 48000 * 5; ++i) pk = std::max (pk, std::abs (in[i]));
        const float peakAfter = 20.0f * std::log10 (pk) + g;
        std::printf ("      limited: gain %.2f dB, sample peak after gain %.2f dBFS\n", g, peakAfter);
        CHECK (done && e.popCalibrationLimited() && peakAfter < -2.0f && peakAfter > -3.6f, "peak guard limits calibration to about -3 dBFS true peak");
    }
    { // Pegel je Kanal: links laut, rechts leise
        GainEngine e; e.prepare (fs); GainParams q;
        std::vector<float> L = sine (fs, 1000, 0.5, 48000), R = sine (fs, 1000, 0.05, 48000);
        for (size_t i = 0; i < L.size(); i += 512)
        {
            float* c[2] = { L.data() + i, R.data() + i };
            e.process (c, 2, (int) std::min<size_t> (512, L.size() - i), q);
        }
        std::printf ("      L rms %.1f peak %.1f | R rms %.1f peak %.1f\n", e.rmsDbCh[0].load(), e.peakDbCh[0].load(), e.rmsDbCh[1].load(), e.peakDbCh[1].load());
        CHECK (std::abs (e.rmsDbCh[0].load() - e.rmsDbCh[1].load() - 20.0f) < 0.5f && e.peakDbCh[0].load() > e.rmsDbCh[0].load(), "per-channel meters");
    }
    std::printf (fails ? "\n%d FAILED\n" : "\nall passed\n", fails);
    return fails ? 1 : 0;
}
