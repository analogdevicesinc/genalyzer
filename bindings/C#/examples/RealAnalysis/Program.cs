// Copyright (C) 2024-2025 Analog Devices, Inc.
// SPDX short identifier: GPL-2.0-or-later
//
// C# port of bindings/python/examples/real_analysis.py
//
// Demonstrates real-signal generation, polynomial distortion, quantization,
// histogram/DNL/INL computation, real FFT, and full Fourier + code-density
// analysis using the Genalyzer C# bindings.
// Run independently: dotnet run --project RealAnalysis.csproj

using System;
using System.Collections.Generic;
using Genalyzer;

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
int    navg       = 2;
int    nfft       = 1024 * 256;
double fs         = 1e9;
double fsr        = 2.0;
double amplDbfs   = -1.0;
double freq       = 10e6;
double phase      = 0.110;
double td         = 0.0;
double tj         = 0.0;
double[] poco     = { 0.0, 1.0, 0.0, 0.003 };
int    qres       = 12;
double qnoiseDbfs = -63.0;
var    codeFmt    = CodeFormat.TwosComplement;
var    sigType    = DnlSignal.Tone;
var    inlFit     = InlLineFit.BestFit;
var    rfftScale  = RfftScale.DbfsSin;
var    window     = Window.NoWindow;

int    npts   = navg * nfft;
double ampl   = (fsr / 2) * Math.Pow(10.0, amplDbfs  / 20.0);
double qnoise = Math.Pow(10.0, qnoiseDbfs / 20.0);
int    ssbFund = 4;
int    ssbRest = 3;

if (window == Window.NoWindow)
{
    freq    = FourierUtilities.Coherent(nfft, fs, freq);
    ssbFund = 0;
    ssbRest = 0;
}

// ---------------------------------------------------------------------------
// Signal generation and processing
// ---------------------------------------------------------------------------
double[] awf    = Waveforms.Cos(npts, fs, ampl, freq, phase, td, tj);
awf             = SignalProcessing.Polyval(awf, poco);
int[]    qwf    = SignalProcessing.Quantize32(awf, fsr, qres, qnoise, codeFmt);
ulong[]  hist   = CodeDensity.Hist(qwf, qres, codeFmt);
double[] dnl    = CodeDensity.Dnl(hist, sigType);
double[] inl    = CodeDensity.Inl(dnl, inlFit);
double[] fftCplx = FourierTransforms.Rfft(qwf, qres, navg, nfft, window, codeFmt, rfftScale);

// ---------------------------------------------------------------------------
// Fourier analysis configuration
// ---------------------------------------------------------------------------
const string key = "fa";
Manager.Remove(key);
FourierAnalysis.Create(key);
FourierAnalysis.SetAnalysisBandExpr(key, "fdata*0.0", "fdata*1.0");
FourierAnalysis.AddFixedTone(key, "A", FACompTag.Signal, freq, ssbFund);
FourierAnalysis.SetHd(key, 3);
FourierAnalysis.SetSsb(key, FASsb.Default, ssbRest);
FourierAnalysis.SetSsb(key, FASsb.DC,      -1);
FourierAnalysis.SetSsb(key, FASsb.Signal,  -1);
FourierAnalysis.SetSsb(key, FASsb.WO,      -1);
FourierAnalysis.SetFsample(key, fs);

Console.WriteLine(FourierAnalysis.Preview(key, false));

// ---------------------------------------------------------------------------
// Analysis
// ---------------------------------------------------------------------------
Dictionary<string, double> wfResults   = Waveforms.WfAnalysis(qwf);
Dictionary<string, double> histResults = CodeDensity.HistAnalysis(hist);
Dictionary<string, double> dnlResults  = CodeDensity.DnlAnalysis(dnl);
Dictionary<string, double> inlResults  = CodeDensity.InlAnalysis(inl);
Dictionary<string, double> fftResults  = FourierAnalysis.Analyze(key, fftCplx, nfft);

// ---------------------------------------------------------------------------
// Print results
// ---------------------------------------------------------------------------
Console.WriteLine("\nWaveform Analysis Results:");
foreach (var kv in wfResults)
    Console.WriteLine($"  {kv.Key,-10} {kv.Value,16:F6}");

Console.WriteLine("\nHistogram Analysis Results:");
foreach (var kv in histResults)
    Console.WriteLine($"  {kv.Key,-10} {kv.Value,16:F6}");

Console.WriteLine("\nDNL Analysis Results:");
foreach (var kv in dnlResults)
    Console.WriteLine($"  {kv.Key,-10} {kv.Value,16:F6}");

Console.WriteLine("\nINL Analysis Results:");
foreach (var kv in inlResults)
    Console.WriteLine($"  {kv.Key,-10} {kv.Value,16:F6}");

Console.WriteLine("\nFourier Analysis Results:");
foreach (var k in new[] { "fsnr", "dc:mag_dbfs", "A:freq", "A:ffinal", "A:mag_dbfs", "A:phase" })
    Console.WriteLine($"  {k,-20} {fftResults[k],20:F6}");
