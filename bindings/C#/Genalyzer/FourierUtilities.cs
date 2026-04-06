// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;

namespace Genalyzer
{
    /// <summary>
    /// Fourier utility functions: alias, coherent frequency, FFT shift,
    /// frequency axis, and inverse FFT shift.
    /// </summary>
    public static class FourierUtilities
    {
        // ---------------------------------------------------------------
        // Size helper
        // ---------------------------------------------------------------

        /// <summary>
        /// Returns the number of elements in a frequency-axis array.
        /// </summary>
        public static int FreqAxisSize(int nfft, FreqAxisType axisType)
        {
            Util.Check(NativeMethods.gn_freq_axis_size(
                out UIntPtr sz, (UIntPtr)nfft, (int)axisType));
            return (int)sz;
        }

        // ---------------------------------------------------------------
        // Utilities
        // ---------------------------------------------------------------

        /// <summary>
        /// Returns the alias of <paramref name="freq"/> given sample rate
        /// <paramref name="fs"/> on the specified frequency-axis type.
        /// </summary>
        public static double Alias(double fs, double freq, FreqAxisType axisType)
        {
            Util.Check(NativeMethods.gn_alias(
                out double output, fs, freq, (int)axisType));
            return output;
        }

        /// <summary>
        /// Returns the nearest coherent frequency for an FFT of size
        /// <paramref name="nfft"/> with sample rate <paramref name="fs"/>.
        /// </summary>
        public static double Coherent(int nfft, double fs, double freq)
        {
            Util.Check(NativeMethods.gn_coherent(
                out double output, (UIntPtr)nfft, fs, freq));
            return output;
        }

        /// <summary>
        /// Shifts the zero-frequency component to the center of the spectrum.
        /// </summary>
        public static double[] FftShift(double[] input)
        {
            var output = new double[input.Length];
            Util.Check(NativeMethods.gn_fftshift(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length));
            return output;
        }

        /// <summary>
        /// Generates a frequency axis array.
        /// </summary>
        public static double[] FreqAxis(int nfft, FreqAxisType axisType,
            double fs = 1.0,
            FreqAxisFormat axisFormat = FreqAxisFormat.Freq)
        {
            int size = FreqAxisSize(nfft, axisType);
            var output = new double[size];
            Util.Check(NativeMethods.gn_freq_axis(
                output, (UIntPtr)size,
                (UIntPtr)nfft, (int)axisType,
                fs, (int)axisFormat));
            return output;
        }

        /// <summary>
        /// Inverse of <see cref="FftShift"/>: shifts the zero-frequency
        /// component back to the beginning.
        /// </summary>
        public static double[] IfftShift(double[] input)
        {
            var output = new double[input.Length];
            Util.Check(NativeMethods.gn_ifftshift(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length));
            return output;
        }
    }
}
