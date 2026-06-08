// Copyright (C) 2024-2025 Analog Devices, Inc.
// SPDX short identifier: GPL-2.0-or-later
//
// C# port of bindings/python/examples/fft_analysis.py
//
// Demonstrates complex I/Q signal generation with quadrature phase error and
// polynomial distortion, frequency shift, downsampling, complex FFT, and full
// Fourier analysis including fixed-tone configuration.
// Run independently: dotnet run --project FftAnalysis.csproj

using System;
using System.Collections.Generic;
using Genalyzer;

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
int    navg        = 2;
int    nfft        = 1024 * 16;
double fs          = 1e9;
double fdata       = fs;
double fshift      = 0.0;
double fsr         = 2.0;
double amplDbfs    = -1.0;
double freq        = 70e6;
double phase       = 0.110;
double td          = 0.0;
double tj          = 0.0;
double qpe         = (Math.PI / 2) * 1e-5;          // quadrature phase error
double[] poco      = { 0.0, 1.0, 0.0, 0.003 };      // distortion polynomial
int    qres        = 12;
double qnoiseDbfs  = -63.0;
var    codeFmt     = CodeFormat.TwosComplement;
var    axisType    = FreqAxisType.DcCenter;
var    window      = Window.NoWindow;

int    dsr         = (int)(fs / fdata);
int    npts        = navg * nfft * dsr;
double ampl        = (fsr / 2) * Math.Pow(10.0, amplDbfs  / 20.0);
double qnoise      = Math.Pow(10.0, qnoiseDbfs / 20.0);
int    ssbFund     = 4;
int    ssbRest     = 3;

if (window == Window.NoWindow)
{
    freq    = FourierUtilities.Coherent(nfft, fdata, freq);
    double fbin = fdata / nfft;
    fshift  = Math.Round(fshift / fbin) * fbin;
    ssbFund = 0;
    ssbRest = 0;
}

// ---------------------------------------------------------------------------
// Signal generation and processing
// ---------------------------------------------------------------------------
double[] awfi = Waveforms.Cos(npts, fs, ampl, freq, phase, td, tj);
double[] awfq = Waveforms.Sin(npts, fs, ampl, freq, phase + qpe, td, tj);
awfi = SignalProcessing.Polyval(awfi, poco);
awfq = SignalProcessing.Polyval(awfq, poco);
int[] qwfi = SignalProcessing.Quantize32(awfi, fsr, qres, qnoise, codeFmt);
int[] qwfq = SignalProcessing.Quantize32(awfq, fsr, qres, qnoise, codeFmt);
int[] xwf  = SignalProcessing.Fshift(qwfi, qwfq, qres, fs, fshift, codeFmt);
xwf = SignalProcessing.Downsample(xwf, dsr, true);
// xwf is interleaved I/Q; split into separate arrays for Fft(int[],int[],...)
int[] xwfI = new int[xwf.Length / 2];
int[] xwfQ = new int[xwf.Length / 2];
for (int idx = 0; idx < xwfI.Length; idx++) { xwfI[idx] = xwf[idx * 2]; xwfQ[idx] = xwf[idx * 2 + 1]; }
double[] fftCplx = FourierTransforms.Fft(xwfI, xwfQ, qres, navg, nfft, window, codeFmt);

// ---------------------------------------------------------------------------
// Fourier analysis configuration
// ---------------------------------------------------------------------------
const string key = "fa";
Manager.Remove(key);
FourierAnalysis.Create(key);
FourierAnalysis.SetAnalysisBandExpr(key, "fdata*0.0", "fdata*1.0");
FourierAnalysis.AddFixedTone(key, "A", FACompTag.Signal, freq, ssbFund);
FourierAnalysis.SetConvOffset(key, fshift != 0.0);
FourierAnalysis.SetHd(key, 3);
FourierAnalysis.SetSsb(key, FASsb.Default, ssbRest);
FourierAnalysis.SetSsb(key, FASsb.DC,      -1);
FourierAnalysis.SetSsb(key, FASsb.Signal,  -1);
FourierAnalysis.SetSsb(key, FASsb.WO,      -1);
FourierAnalysis.SetFdata(key, fdata);
FourierAnalysis.SetFsample(key, fs);
FourierAnalysis.SetFshift(key, fshift);

Console.WriteLine(FourierAnalysis.Preview(key, true));

// ---------------------------------------------------------------------------
// Fourier analysis execution
// ---------------------------------------------------------------------------
Dictionary<string, double> results = FourierAnalysis.Analyze(key, fftCplx, nfft, axisType);

string carrier = FourierAnalysis.GetResultString(results, "carrierindex");
string maxspur = FourierAnalysis.GetResultString(results, "maxspurindex");

// ---------------------------------------------------------------------------
// Print results
// ---------------------------------------------------------------------------
foreach (var k in new[] { "fsnr", "sfdr", "dc:mag_dbfs", "A:freq", "A:ffinal",
                           "A:mag_dbfs", "A:phase", "-3A:mag_dbc" })
    Console.WriteLine($"{k,-20} {results[k],20:F6}");

Console.WriteLine($"{"Carrier",-20} {carrier,20}");
Console.WriteLine($"{"MaxSpur",-20} {maxspur,20}");
