// Copyright (C) 2024-2025 Analog Devices, Inc.
// SPDX short identifier: GPL-2.0-or-later
//
// C# port of bindings/python/examples/simplified_beta/gen_tone_and_quantize.py
//
// Uses the simplified-beta GenalyzerConfig API to generate and quantize both
// a real multi-tone and a complex multi-tone waveform.
// Run independently: dotnet run --project GenToneAndQuantize.csproj

using System;
using Genalyzer;

int      npts      = 16384;
double   fs        = 900_000_000.0;
double[] toneFreq  = { 250_000.0, 4_750_000.0, 14_750_000.0 };
double[] toneAmpl  = { 0.25, 1.0, 1.0 };
double[] tonePhase = { 0.3, 0.3, 0.2 };
double   fsr       = 3.0;
int      qres      = 12;
double   qnoise    = 0.0;

// ?? Real tone ???????????????????????????????????????????????????????????????
using (var cfg1 = new GenalyzerConfig())
{
    cfg1.ConfigGenTone(ToneType.RealCosine, npts, fs,
        toneFreq, toneAmpl, tonePhase);
    cfg1.ConfigQuantize(npts, fsr, qres, qnoise);

    double[] awf1 = cfg1.GenRealTone();
    int[]    qwf1 = cfg1.Quantize(awf1);

    Console.WriteLine("Real tone - first 10 analog samples:");
    for (int i = 0; i < 10; i++)
        Console.WriteLine($"  awf1[{i}] = {awf1[i]:F6}");

    Console.WriteLine("\nReal tone - first 10 quantized samples:");
    for (int i = 0; i < 10; i++)
        Console.WriteLine($"  qwf1[{i}] = {qwf1[i]}");
}

// ?? Complex tone ????????????????????????????????????????????????????????????
using (var cfg2 = new GenalyzerConfig())
{
    cfg2.ConfigGenTone(ToneType.ComplexExp, npts, fs,
        toneFreq, toneAmpl, tonePhase);
    cfg2.ConfigQuantize(npts, fsr, qres, qnoise);

    (double[] awf2I, double[] awf2Q) = cfg2.GenComplexTone();
    int[] qwf2I = cfg2.Quantize(awf2I);
    int[] qwf2Q = cfg2.Quantize(awf2Q);

    Console.WriteLine("\nComplex tone - first 10 analog I samples:");
    for (int i = 0; i < 10; i++)
        Console.WriteLine($"  awf2I[{i}] = {awf2I[i]:F6}");

    Console.WriteLine("\nComplex tone - first 10 analog Q samples:");
    for (int i = 0; i < 10; i++)
        Console.WriteLine($"  awf2Q[{i}] = {awf2Q[i]:F6}");

    Console.WriteLine("\nComplex tone - first 10 quantized I samples:");
    for (int i = 0; i < 10; i++)
        Console.WriteLine($"  qwf2I[{i}] = {qwf2I[i]}");

    Console.WriteLine("\nComplex tone - first 10 quantized Q samples:");
    for (int i = 0; i < 10; i++)
        Console.WriteLine($"  qwf2Q[{i}] = {qwf2Q[i]}");
}
