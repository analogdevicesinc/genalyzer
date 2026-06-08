// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;

namespace Genalyzer
{
    /// <summary>
    /// Signal processing: downsample, frequency shift, normalize,
    /// polynomial distortion, and quantize.
    /// </summary>
    public static class SignalProcessing
    {
        // ---------------------------------------------------------------
        // Size helpers
        // ---------------------------------------------------------------

        /// <summary>Returns the output array size for a downsample operation.</summary>
        public static int DownsampleSize(int inSize, int ratio, bool interleaved)
        {
            Util.Check(NativeMethods.gn_downsample_size(
                out UIntPtr sz, (UIntPtr)inSize, ratio, interleaved));
            return (int)sz;
        }

        /// <summary>Returns the output array size for a frequency-shift operation.</summary>
        public static int FshiftSize(int iSize, int qSize)
        {
            Util.Check(NativeMethods.gn_fshift_size(
                out UIntPtr sz, (UIntPtr)iSize, (UIntPtr)qSize));
            return (int)sz;
        }

        // ---------------------------------------------------------------
        // Downsample
        // ---------------------------------------------------------------

        /// <summary>Downsamples a normalized double array.</summary>
        public static double[] Downsample(double[] input, int ratio,
            bool interleaved = false)
        {
            int outSize = DownsampleSize(input.Length, ratio, interleaved);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_downsample(
                output, (UIntPtr)outSize,
                input,  (UIntPtr)input.Length,
                ratio, interleaved));
            return output;
        }

        /// <summary>Downsamples a 16-bit integer array.</summary>
        public static short[] Downsample(short[] input, int ratio,
            bool interleaved = false)
        {
            int outSize = DownsampleSize(input.Length, ratio, interleaved);
            var output  = new short[outSize];
            Util.Check(NativeMethods.gn_downsample16(
                output, (UIntPtr)outSize,
                input,  (UIntPtr)input.Length,
                ratio, interleaved));
            return output;
        }

        /// <summary>Downsamples a 32-bit integer array.</summary>
        public static int[] Downsample(int[] input, int ratio,
            bool interleaved = false)
        {
            int outSize = DownsampleSize(input.Length, ratio, interleaved);
            var output  = new int[outSize];
            Util.Check(NativeMethods.gn_downsample32(
                output, (UIntPtr)outSize,
                input,  (UIntPtr)input.Length,
                ratio, interleaved));
            return output;
        }

        /// <summary>Downsamples a 64-bit integer array.</summary>
        public static long[] Downsample(long[] input, int ratio,
            bool interleaved = false)
        {
            int outSize = DownsampleSize(input.Length, ratio, interleaved);
            var output  = new long[outSize];
            Util.Check(NativeMethods.gn_downsample64(
                output, (UIntPtr)outSize,
                input,  (UIntPtr)input.Length,
                ratio, interleaved));
            return output;
        }

        // ---------------------------------------------------------------
        // Frequency shift
        // ---------------------------------------------------------------

        /// <summary>
        /// Frequency-shifts a split normalized double I/Q pair.
        /// Returns an interleaved output array.
        /// </summary>
        public static double[] Fshift(double[] i, double[] q,
            double fs, double fshift)
        {
            int outSize = FshiftSize(i.Length, q.Length);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_fshift(
                output, (UIntPtr)outSize,
                i, (UIntPtr)i.Length,
                q, (UIntPtr)q.Length,
                fs, fshift));
            return output;
        }

        /// <summary>Frequency-shifts split 16-bit I/Q arrays.</summary>
        public static short[] Fshift(short[] i, short[] q,
            int n, double fs, double fshift,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            int outSize = FshiftSize(i.Length, q.Length);
            var output  = new short[outSize];
            Util.Check(NativeMethods.gn_fshift16(
                output, (UIntPtr)outSize,
                i, (UIntPtr)i.Length,
                q, (UIntPtr)q.Length,
                n, fs, fshift, (int)format));
            return output;
        }

        /// <summary>Frequency-shifts split 32-bit I/Q arrays.</summary>
        public static int[] Fshift(int[] i, int[] q,
            int n, double fs, double fshift,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            int outSize = FshiftSize(i.Length, q.Length);
            var output  = new int[outSize];
            Util.Check(NativeMethods.gn_fshift32(
                output, (UIntPtr)outSize,
                i, (UIntPtr)i.Length,
                q, (UIntPtr)q.Length,
                n, fs, fshift, (int)format));
            return output;
        }

        /// <summary>Frequency-shifts split 64-bit I/Q arrays.</summary>
        public static long[] Fshift(long[] i, long[] q,
            int n, double fs, double fshift,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            int outSize = FshiftSize(i.Length, q.Length);
            var output  = new long[outSize];
            Util.Check(NativeMethods.gn_fshift64(
                output, (UIntPtr)outSize,
                i, (UIntPtr)i.Length,
                q, (UIntPtr)q.Length,
                n, fs, fshift, (int)format));
            return output;
        }

        // ---------------------------------------------------------------
        // Normalize
        // ---------------------------------------------------------------

        /// <summary>Normalizes a 16-bit integer array to [-1, 1] doubles.</summary>
        public static double[] Normalize(short[] input, int n,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            var output = new double[input.Length];
            Util.Check(NativeMethods.gn_normalize16(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length,
                n, (int)format));
            return output;
        }

        /// <summary>Normalizes a 32-bit integer array to [-1, 1] doubles.</summary>
        public static double[] Normalize(int[] input, int n,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            var output = new double[input.Length];
            Util.Check(NativeMethods.gn_normalize32(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length,
                n, (int)format));
            return output;
        }

        /// <summary>Normalizes a 64-bit integer array to [-1, 1] doubles.</summary>
        public static double[] Normalize(long[] input, int n,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            var output = new double[input.Length];
            Util.Check(NativeMethods.gn_normalize64(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length,
                n, (int)format));
            return output;
        }

        // ---------------------------------------------------------------
        // Polynomial distortion
        // ---------------------------------------------------------------

        /// <summary>
        /// Evaluates a polynomial with coefficients <paramref name="c"/> at
        /// each point in <paramref name="input"/>.
        /// </summary>
        public static double[] Polyval(double[] input, double[] c)
        {
            var output = new double[input.Length];
            Util.Check(NativeMethods.gn_polyval(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length,
                c,      (UIntPtr)c.Length));
            return output;
        }

        // ---------------------------------------------------------------
        // Quantize
        // ---------------------------------------------------------------

        /// <summary>Quantizes a double waveform to 16-bit integers.</summary>
        public static short[] Quantize16(double[] input,
            double fsr, int n, double noise = 0.0,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            var output = new short[input.Length];
            Util.Check(NativeMethods.gn_quantize16(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length,
                fsr, n, noise, (int)format));
            return output;
        }

        /// <summary>Quantizes a double waveform to 32-bit integers.</summary>
        public static int[] Quantize32(double[] input,
            double fsr, int n, double noise = 0.0,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            var output = new int[input.Length];
            Util.Check(NativeMethods.gn_quantize32(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length,
                fsr, n, noise, (int)format));
            return output;
        }

        /// <summary>Quantizes a double waveform to 64-bit integers.</summary>
        public static int[] Quantize64(double[] input,
            double fsr, int n, double noise = 0.0,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            // gn_quantize64 produces long[] but the managed API mirrors the
            // C API which writes int64_t; return as int[] via a copy so the
            // caller can use the same type as Quantize32 for n<=32.
            var raw    = new long[input.Length];
            var output = new int[input.Length];
            Util.Check(NativeMethods.gn_quantize64(
                raw, (UIntPtr)raw.Length,
                input, (UIntPtr)input.Length,
                fsr, n, noise, (int)format));
            // expose as long[] to preserve full precision
            // (returned as int[] only when used for small n; provide long overload)
            for (int i = 0; i < raw.Length; i++) output[i] = (int)raw[i];
            return output;
        }

        /// <summary>Quantizes a double waveform to 64-bit integers (full precision).</summary>
        public static long[] Quantize64Long(double[] input,
            double fsr, int n, double noise = 0.0,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            var output = new long[input.Length];
            Util.Check(NativeMethods.gn_quantize64(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length,
                fsr, n, noise, (int)format));
            return output;
        }
    }
}
