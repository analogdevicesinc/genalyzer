// Copyright (C) 2024-2025 Analog Devices, Inc.
// SPDX short identifier: GPL-2.0-or-later
//
// C# port of bindings/python/examples/gn_doc_spectral_analysis2.py
//
// Same as SpectralAnalysis1 but with a Blackman-Harris window and a tone
// frequency that is not coherent, so ssb_fund > 0.
// Run independently: dotnet run --project SpectralAnalysis2.csproj

using System;
using System.Collections.Generic;
using Genalyzer;

int    npts        = 30000;
double freq        = 375_000.0;
double phase       = 0.0;
double amplDbfs    = -1.0;
double qnoiseDbfs  = -60.0;
double fsr         = 2.0;
double ampl        = (fsr / 2) * Math.Pow(10.0, amplDbfs / 20.0);
double qnoise      = Math.Pow(10.0, qnoiseDbfs / 20.0);

int    navg         = 1;
int    nfft         = npts / navg;
int    qres         = 12;
var    codeFmt      = CodeFormat.TwosComplement;
var    window       = Window.BlackmanHarris;
int    ssbFund      = 6;
var    axisType     = FreqAxisType.DcCenter;
double fs           = 4e6;
int    numHarmonics = 3;
int    ssbRest      = 0;
int    ssbDc        = 0;
int    ssbWo        = 0;

double[] awfi = Waveforms.Cos(npts, fs, ampl, freq, phase);
double[] awfq = Waveforms.Sin(npts, fs, ampl, freq, phase);
short[]  qwfi = SignalProcessing.Quantize16(awfi, fsr, qres, qnoise, codeFmt);
short[]  qwfq = SignalProcessing.Quantize16(awfq, fsr, qres, qnoise, codeFmt);

double[] fftCplx = FourierTransforms.Fft(qwfi, qwfq, qres, navg, nfft, window, codeFmt);

const string key = "fa";
Manager.Remove(key);
FourierAnalysis.Create(key);
FourierAnalysis.AddMaxTone(key, "A", FACompTag.Signal, ssbFund);
FourierAnalysis.SetFsample(key, fs);
FourierAnalysis.SetHd(key, numHarmonics);
FourierAnalysis.SetSsb(key, FASsb.Default, ssbRest);
FourierAnalysis.SetSsb(key, FASsb.DC,      ssbDc);
FourierAnalysis.SetSsb(key, FASsb.WO,      ssbWo);

Console.WriteLine(FourierAnalysis.Preview(key, true));

Dictionary<string, double> results = FourierAnalysis.Analyze(key, fftCplx, nfft, axisType);

Console.WriteLine("+----------------+");
Console.WriteLine("results dictionary");
Console.WriteLine("+----------------+");
foreach (var kv in results)
    Console.WriteLine($"  {kv.Key,-26} {kv.Value,16:F6}");

Console.WriteLine();
Console.WriteLine($"SNR   = {results["snr"],8:F4} dB");
Console.WriteLine($"FSNR  = {results["fsnr"],8:F4} dB");
Console.WriteLine($"SINAD = {results["sinad"],8:F4} dB");
Console.WriteLine($"SFDR  = {results["sfdr"],8:F4} dB");
Console.WriteLine($"ABN   = {results["abn"],8:F4} dB");
Console.WriteLine($"NSD   = {results["nsd"],8:F4} dB");
