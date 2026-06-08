// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Collections.Generic;
using Genalyzer;
using Xunit;

namespace Genalyzer.Tests
{
    [Collection("Genalyzer")]
    public sealed class WaveformsTests
    {
        [Fact]
        public void Sin_ReturnsRequestedLength()
        {
            double[] wf = Waveforms.Sin(1024, fs: 1000.0, ampl: 1.0, freq: 1.0);
            Assert.Equal(1024, wf.Length);
        }

        [Fact]
        public void Sin_StartsAtZero_AndIsBounded()
        {
            // sin(0) == 0; a pure tone of amplitude A stays within [-A, A].
            const double ampl = 0.75;
            double[] wf = Waveforms.Sin(1000, fs: 1000.0, ampl: ampl, freq: 1.0);

            Assert.Equal(0.0, wf[0], precision: 9);
            foreach (double v in wf)
                Assert.InRange(v, -ampl - 1e-9, ampl + 1e-9);
        }

        [Fact]
        public void Cos_StartsAtAmplitude()
        {
            // cos(0) == 1, so the first sample equals the amplitude.
            const double ampl = 1.5;
            double[] wf = Waveforms.Cos(1000, fs: 1000.0, ampl: ampl, freq: 1.0);

            Assert.Equal(ampl, wf[0], precision: 9);
        }

        [Fact]
        public void Ramp_IsMonotonicFromStartToStop()
        {
            double[] wf = Waveforms.Ramp(100, start: 0.0, stop: 99.0);

            Assert.Equal(100, wf.Length);
            Assert.True(wf[0] <= wf[^1]);
            for (int i = 1; i < wf.Length; i++)
                Assert.True(wf[i] >= wf[i - 1]);
        }

        [Fact]
        public void WfAnalysis_ReturnsExpectedKeys_AndRange()
        {
            // A full-scale sine should report min ~ -A and max ~ +A. This also
            // exercises the native-string (result-key) UTF-8 decode path.
            const double ampl = 1.0;
            double[] wf = Waveforms.Sin(8192, fs: 8192.0, ampl: ampl, freq: 64.0);

            Dictionary<string, double> r = Waveforms.WfAnalysis(wf);

            Assert.NotEmpty(r);
            Assert.Contains("min", r.Keys);
            Assert.Contains("max", r.Keys);
            Assert.Equal(ampl, r["max"], precision: 3);
            Assert.Equal(-ampl, r["min"], precision: 3);
            // Every decoded key must be a clean, non-empty ASCII/UTF-8 string.
            foreach (string key in r.Keys)
                Assert.False(string.IsNullOrEmpty(key));
        }
    }
}
