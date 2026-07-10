// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using Genalyzer;
using Xunit;

namespace Genalyzer.Tests
{
    [Collection("Genalyzer")]
    public sealed class GenalyzerConfigTests
    {
        [Fact]
        public void GenRealTone_SingleCosine_MatchesWaveformsCos()
        {
            // A single-tone REAL_COSINE generation must reproduce exactly what
            // Waveforms.Cos produces for the same parameters (the native path
            // for one tone reduces to gn_cos).
            const int npts = 4096;
            const double fs = 1_000_000.0;
            const double freq = 12_500.0;
            const double ampl = 0.8;
            const double phase = 0.25;

            using var cfg = new GenalyzerConfig();
            cfg.ConfigGenTone(ToneType.RealCosine, npts, fs,
                new[] { freq }, new[] { ampl }, new[] { phase });

            // Force a GC (with compaction) BETWEEN configuration and generation.
            // The native config retains the tone-array pointers; if the binding
            // did not pin them for the object's lifetime, this would surface the
            // dangling-pointer bug as corrupted output.
            GC.Collect();
            GC.WaitForPendingFinalizers();
            GC.Collect();

            double[] generated = cfg.GenRealTone();
            double[] expected = Waveforms.Cos(npts, fs, ampl, freq, phase);

            Assert.Equal(npts, generated.Length);
            for (int i = 0; i < npts; i++)
                Assert.Equal(expected[i], generated[i], precision: 9);
        }

        [Fact]
        public void GenComplexTone_ReturnsTwoEqualLengthBoundedChannels()
        {
            const int npts = 2048;
            const double fs = 3_000_000.0;

            using var cfg = new GenalyzerConfig();
            cfg.ConfigGenTone(ToneType.ComplexExp, npts, fs,
                new[] { 300_000.0 }, new[] { 0.9 }, new[] { 0.0 });

            GC.Collect();
            GC.WaitForPendingFinalizers();

            (double[] i, double[] q) = cfg.GenComplexTone();

            Assert.Equal(npts, i.Length);
            Assert.Equal(npts, q.Length);
            foreach (double v in i) Assert.InRange(v, -0.9 - 1e-9, 0.9 + 1e-9);
            foreach (double v in q) Assert.InRange(v, -0.9 - 1e-9, 0.9 + 1e-9);
        }

        [Fact]
        public void Quantize_ProducesCodesWithinResolution()
        {
            const int npts = 1024;
            const double fs = 1_000_000.0;
            const int qres = 12;
            const double fsr = 2.0;

            using var cfg = new GenalyzerConfig();
            cfg.ConfigGenTone(ToneType.RealCosine, npts, fs,
                new[] { 10_000.0 }, new[] { 1.0 }, new[] { 0.0 });
            cfg.ConfigQuantize(npts, fsr, qres, 0.0);

            double[] awf = cfg.GenRealTone();
            int[] codes = cfg.Quantize(awf);

            Assert.Equal(npts, codes.Length);
            int maxCode = 1 << qres; // generous upper bound on |code|
            foreach (int c in codes)
                Assert.InRange(c, -maxCode, maxCode);
        }

        [Fact]
        public void SetToneFreq_ReplacingArray_DoesNotLeakOrCorrupt()
        {
            // Calling a tone setter twice must free the first pin and pin the
            // second array; the later generation must reflect the second call.
            const int npts = 1024;
            const double fs = 1_000_000.0;

            using var cfg = new GenalyzerConfig();
            cfg.SetToneType(ToneType.RealCosine);
            cfg.SetNpts(npts);
            cfg.SetSampleRate(fs);
            cfg.SetNumTones(1);
            cfg.SetToneAmpl(new[] { 1.0 });
            cfg.SetTonePhase(new[] { 0.0 });

            cfg.SetToneFreq(new[] { 10_000.0 });
            cfg.SetToneFreq(new[] { 25_000.0 }); // replaces the first pin

            GC.Collect();
            GC.WaitForPendingFinalizers();

            double[] generated = cfg.GenRealTone();
            double[] expected = Waveforms.Cos(npts, fs, 1.0, 25_000.0, 0.0);

            for (int i = 0; i < npts; i++)
                Assert.Equal(expected[i], generated[i], precision: 9);
        }
    }
}
