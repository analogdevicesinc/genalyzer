// Copyright (C) 2024-2025 Analog Devices, Inc.
// SPDX short identifier: GPL-2.0-or-later
//
// C# port of bindings/python/examples/simplified_beta/do_histogram_analysis.py
//
// Uses the simplified-beta GenalyzerConfig API to generate a ramp, quantize
// it, compute a histogram, and run histogram analysis.
// Run independently: dotnet run --project HistogramAnalysis.csproj

using System;
using System.Collections.Generic;
using Genalyzer;

int    npts       = 8192;
double rampStart  = 0.0;
double rampStop   = 2.0;
double fsr        = 3.0;
int    qres       = 12;
double qnoise     = Math.Pow(10.0, -60.0 / 20.0);

using var cfg = new GenalyzerConfig();
cfg.ConfigGenRamp(npts, rampStart, rampStop);
cfg.ConfigQuantize(npts, fsr, qres, qnoise);

double[] awf  = cfg.GenRamp();
int[]    qwf  = cfg.Quantize(awf);

cfg.ConfigHistzNla(npts, qres);
ulong[] hist = cfg.Histz(qwf);

Dictionary<string, double> results = CodeDensity.HistAnalysis(hist);

Console.WriteLine("Histogram Analysis Results:");
foreach (var kv in results)
    Console.WriteLine($"  {kv.Key,-16} {kv.Value,16:F6}");
