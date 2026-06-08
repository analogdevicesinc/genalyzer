// Copyright (C) 2024-2025 Analog Devices, Inc.
// SPDX short identifier: GPL-2.0-or-later
//
// C# port of bindings/python/examples/simplified_beta/do_waveform_analysis.py
//
// Uses the simplified-beta GenalyzerConfig API to generate a real tone,
// quantize it, and run waveform analysis.
// Run independently: dotnet run --project WaveformAnalysis.csproj

using System;
using System.Collections.Generic;
using Genalyzer;

int    npts    = 8192;
double fs      = 5_000_000.0;
double freq    = 50_000.0;
double ampl    = 0.5;
double phase   = 0.2;
double fsr     = 3.0;
int    qres    = 12;
double qnoise  = Math.Pow(10.0, -60.0 / 20.0);

using var cfg = new GenalyzerConfig();
cfg.ConfigGenTone(ToneType.RealCosine, npts, fs,
new[] { freq }, new[] { ampl }, new[] { phase });
cfg.ConfigQuantize(npts, fsr, qres, qnoise);

double[] awf = cfg.GenRealTone();
int[]    qwf = cfg.Quantize(awf);

Dictionary<string, double> results = Waveforms.WfAnalysis(qwf);

Console.WriteLine("Waveform Analysis Results:");
foreach (var kv in results)
    Console.WriteLine($"  {kv.Key,-14} {kv.Value,16:F6}");
