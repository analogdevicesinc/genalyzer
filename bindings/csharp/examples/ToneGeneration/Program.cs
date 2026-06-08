// Copyright (C) 2024-2025 Analog Devices, Inc.
// SPDX short identifier: GPL-2.0-or-later
//
// C# port of bindings/python/examples/gn_doc_tone_gen.py
//
// Demonstrates complex I/Q tone generation, quantization, and waveform
// analysis using the Genalyzer C# bindings.
// Run independently: dotnet run --project ToneGeneration.csproj

using System;
using System.Collections.Generic;
using Genalyzer;

// ---------------------------------------------------------------------------
// Signal parameters
// ---------------------------------------------------------------------------
int    npts        = 30000;           // number of samples
double fs          = 3e6;             // sample rate (S/s)
double freq        = 300_000.0;       // tone frequency (Hz)
double phase       = 0.0;             // tone phase (rad)
double amplDbfs    = -1.0;            // amplitude in dBFS
double qnoiseDbfs  = -60.0;           // quantizer noise in dBFS
double fsr         = 2.0;             // full-scale range
double ampl        = (fsr / 2) * Math.Pow(10.0, amplDbfs  / 20.0);
double qnoise      = Math.Pow(10.0, qnoiseDbfs / 20.0);
int    qres        = 12;              // quantizer resolution
var    codeFmt     = CodeFormat.TwosComplement;

// ---------------------------------------------------------------------------
// Generate I/Q waveforms and quantize
// ---------------------------------------------------------------------------
double[] awfi = Waveforms.Cos(npts, fs, ampl, freq, phase);
double[] awfq = Waveforms.Sin(npts, fs, ampl, freq, phase);
short[]  qwfi = SignalProcessing.Quantize16(awfi, fsr, qres, qnoise, codeFmt);
short[]  qwfq = SignalProcessing.Quantize16(awfq, fsr, qres, qnoise, codeFmt);

// ---------------------------------------------------------------------------
// Waveform analysis
// ---------------------------------------------------------------------------
Dictionary<string, double> wfaI = Waveforms.WfAnalysis(awfi);
Dictionary<string, double> wfaQ = Waveforms.WfAnalysis(awfq);

Console.WriteLine("+---------------------------+");
Console.WriteLine("waveform-analysis results (I)");
Console.WriteLine("+---------------------------+");
foreach (var kv in wfaI)
    Console.WriteLine($"  {kv.Key,-12} {kv.Value,16:F6}");

Console.WriteLine("+---------------------------+");
Console.WriteLine("waveform-analysis results (Q)");
Console.WriteLine("+---------------------------+");
foreach (var kv in wfaQ)
    Console.WriteLine($"  {kv.Key,-12} {kv.Value,16:F6}");
