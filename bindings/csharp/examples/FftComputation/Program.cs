// Copyright (C) 2024-2025 Analog Devices, Inc.
// SPDX short identifier: GPL-2.0-or-later
//
// C# port of bindings/python/examples/gn_doc_fft.py
//
// Demonstrates I/Q tone generation, quantization, and complex FFT computation
// using the Genalyzer C# bindings.
// Run independently: dotnet run --project FftComputation.csproj

using System;
using Genalyzer;

// ---------------------------------------------------------------------------
// Signal parameters
// ---------------------------------------------------------------------------
int    npts       = 30000;          // number of samples
double fs         = 3e6;            // sample rate (S/s)
double freq       = 300_000.0;      // tone frequency (Hz)
double phase      = 0.0;            // tone phase (rad)
double amplDbfs   = -1.0;           // amplitude in dBFS
double qnoiseDbfs = -60.0;          // quantizer noise in dBFS
double fsr        = 2.0;            // full-scale range
double ampl       = (fsr / 2) * Math.Pow(10.0, amplDbfs  / 20.0);
double qnoise     = Math.Pow(10.0, qnoiseDbfs / 20.0);
int    qres       = 12;             // quantizer resolution
var    codeFmt    = CodeFormat.TwosComplement;

// ---------------------------------------------------------------------------
// FFT configuration
// ---------------------------------------------------------------------------
int         navg     = 1;
int         nfft     = npts / navg;
var         window   = Window.NoWindow;
var         axisType = FreqAxisType.DcCenter;
var         axisFmt  = FreqAxisFormat.Freq;

// ---------------------------------------------------------------------------
// Generate signal and compute FFT
// ---------------------------------------------------------------------------
double[] awfi = Waveforms.Cos(npts, fs, ampl, freq, phase);
double[] awfq = Waveforms.Sin(npts, fs, ampl, freq, phase);
short[]  qwfi = SignalProcessing.Quantize16(awfi, fsr, qres, qnoise, codeFmt);
short[]  qwfq = SignalProcessing.Quantize16(awfq, fsr, qres, qnoise, codeFmt);

double[] fftCplx  = FourierTransforms.Fft(qwfi, qwfq, qres, navg, nfft, window, codeFmt);
double[] freqAxis = FourierUtilities.FreqAxis(nfft, axisType, fs, axisFmt);
double[] fftDb    = ArrayOps.Db(fftCplx);

if (axisType == FreqAxisType.DcCenter)
    fftDb = FourierUtilities.FftShift(fftDb);

// ---------------------------------------------------------------------------
// Print a brief summary (first and last five bins)
// ---------------------------------------------------------------------------
Console.WriteLine($"FFT size : {nfft}");
Console.WriteLine($"Freq axis: [{freqAxis[0]:F0}, {freqAxis[^1]:F0}] Hz");
Console.WriteLine();
Console.WriteLine($"{"Freq (Hz)",14}  {"dBFS",10}");
Console.WriteLine(new string('-', 28));
for (int i = 0; i < Math.Min(5, fftDb.Length); i++)
    Console.WriteLine($"{freqAxis[i],14:F1}  {fftDb[i],10:F3}");
Console.WriteLine("...");
for (int i = fftDb.Length - 5; i < fftDb.Length; i++)
    Console.WriteLine($"{freqAxis[i],14:F1}  {fftDb[i],10:F3}");
