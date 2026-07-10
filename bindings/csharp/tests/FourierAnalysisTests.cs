// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Collections.Generic;
using System.Linq;
using Genalyzer;
using Xunit;

namespace Genalyzer.Tests
{
    [Collection("Genalyzer")]
    public sealed class FourierAnalysisTests
    {
        // Builds a quantized complex tone, FFTs it, and returns both the
        // config (for single-result queries) and the full result dictionary.
        private static Dictionary<string, double> RunSimplifiedFa(
            out double singleSfdr)
        {
            const int npts = 16384;
            const int navg = 1;
            const int nfft = npts / navg;
            const int qres = 12;
            const double fs = 3_000_000.0;
            const double freq = 300_000.0;
            const double ampl = 0.9;

            using var cfg = new GenalyzerConfig();
            cfg.ConfigGenTone(ToneType.ComplexExp, npts, fs,
                new[] { freq }, new[] { ampl }, new[] { 0.0 });
            cfg.ConfigQuantize(npts, 2.0, qres, 0.0);
            cfg.ConfigFftz(npts, qres, navg, nfft, Window.NoWindow);
            cfg.SetSampleRate(fs);
            cfg.ConfigFa(freq);

            (double[] awfI, double[] awfQ) = cfg.GenComplexTone();
            int[] qwfI = cfg.Quantize(awfI);
            int[] qwfQ = cfg.Quantize(awfQ);
            double[] fftIlv = cfg.Fftz(qwfI, qwfQ);

            singleSfdr = cfg.GetFaSingleResult(fftIlv, "sfdr");
            return cfg.GetFaResults(fftIlv);
        }

        [Fact]
        public void SimplifiedFourierAnalysis_ReturnsFiniteMetrics()
        {
            Dictionary<string, double> results = RunSimplifiedFa(out double sfdr);

            Assert.NotEmpty(results);
            Assert.Contains("sfdr", results.Keys);
            Assert.Contains("fsnr", results.Keys);

            // SFDR for a clean 12-bit tone is a large, finite positive dB value.
            Assert.True(double.IsFinite(sfdr));
            Assert.InRange(sfdr, 1.0, 200.0);

            // The single-result query must agree with the full dictionary.
            Assert.Equal(results["sfdr"], sfdr, precision: 6);

            // All keys decode to clean, non-empty strings (UTF-8 decode path).
            Assert.All(results.Keys, k => Assert.False(string.IsNullOrEmpty(k)));
        }

        [Fact]
        public void GetResult_RoundTripsDictionaryValue()
        {
            // Exercises Util.DictionaryToKeyValueArrays (managed dict -> pinned
            // parallel key/value arrays) and gn_fa_result lookup.
            Dictionary<string, double> results = RunSimplifiedFa(out _);
            string key = results.Keys.First();

            double looked = FourierAnalysis.GetResult(results, key);
            Assert.Equal(results[key], looked, precision: 9);
        }
    }
}
