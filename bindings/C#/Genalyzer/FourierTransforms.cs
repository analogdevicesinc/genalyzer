// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;

namespace Genalyzer
{
    /// <summary>
    /// Fourier transforms: complex FFT and real FFT for normalized (double)
    /// and quantized (int16/int32/int64) input data.
    /// All output arrays contain interleaved Re/Im doubles.
    /// </summary>
    public static class FourierTransforms
    {
        // ---------------------------------------------------------------
        // Size helpers
        // ---------------------------------------------------------------

        /// <summary>
        /// Returns the output array length for a complex FFT.
        /// </summary>
        public static int FftSize(int iSize, int qSize, int navg, int nfft)
        {
            Util.Check(NativeMethods.gn_fft_size(
                out UIntPtr sz,
                (UIntPtr)iSize, (UIntPtr)qSize,
                (UIntPtr)navg,  (UIntPtr)nfft));
            return (int)sz;
        }

        /// <summary>
        /// Returns the output array length for a real FFT.
        /// </summary>
        public static int RfftSize(int inSize, int navg, int nfft)
        {
            Util.Check(NativeMethods.gn_rfft_size(
                out UIntPtr sz,
                (UIntPtr)inSize, (UIntPtr)navg, (UIntPtr)nfft));
            return (int)sz;
        }

        // ---------------------------------------------------------------
        // Complex FFT  –  normalized double input
        // ---------------------------------------------------------------

        /// <summary>
        /// Computes the complex FFT of split normalized I/Q double arrays.
        /// </summary>
        public static double[] Fft(double[] i, double[] q,
            int navg, int nfft,
            Window window = Window.NoWindow)
        {
            int outSize = FftSize(i.Length, q.Length, navg, nfft);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_fft(
                output, (UIntPtr)outSize,
                i, (UIntPtr)i.Length,
                q, (UIntPtr)q.Length,
                (UIntPtr)navg, (UIntPtr)nfft, (int)window));
            return output;
        }

        // ---------------------------------------------------------------
        // Complex FFT  –  quantized input
        // ---------------------------------------------------------------

        /// <summary>
        /// Computes the complex FFT of split 16-bit quantized I/Q arrays.
        /// </summary>
        public static double[] Fft(short[] i, short[] q,
            int n, int navg, int nfft,
            Window window = Window.NoWindow,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            int outSize = FftSize(i.Length, q.Length, navg, nfft);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_fft16(
                output, (UIntPtr)outSize,
                i, (UIntPtr)i.Length,
                q, (UIntPtr)q.Length,
                n, (UIntPtr)navg, (UIntPtr)nfft,
                (int)window, (int)format));
            return output;
        }

        /// <summary>
        /// Computes the complex FFT of split 32-bit quantized I/Q arrays.
        /// </summary>
        public static double[] Fft(int[] i, int[] q,
            int n, int navg, int nfft,
            Window window = Window.NoWindow,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            int outSize = FftSize(i.Length, q.Length, navg, nfft);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_fft32(
                output, (UIntPtr)outSize,
                i, (UIntPtr)i.Length,
                q, (UIntPtr)q.Length,
                n, (UIntPtr)navg, (UIntPtr)nfft,
                (int)window, (int)format));
            return output;
        }

        /// <summary>
        /// Computes the complex FFT of split 64-bit quantized I/Q arrays.
        /// </summary>
        public static double[] Fft(long[] i, long[] q,
            int n, int navg, int nfft,
            Window window = Window.NoWindow,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            int outSize = FftSize(i.Length, q.Length, navg, nfft);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_fft64(
                output, (UIntPtr)outSize,
                i, (UIntPtr)i.Length,
                q, (UIntPtr)q.Length,
                n, (UIntPtr)navg, (UIntPtr)nfft,
                (int)window, (int)format));
            return output;
        }

        // ---------------------------------------------------------------
        // Real FFT  –  normalized double input
        // ---------------------------------------------------------------

        /// <summary>
        /// Computes the real FFT of a normalized double array.
        /// </summary>
        public static double[] Rfft(double[] input,
            int navg, int nfft,
            Window window = Window.NoWindow,
            RfftScale scale = RfftScale.DbfsSin)
        {
            int outSize = RfftSize(input.Length, navg, nfft);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_rfft(
                output, (UIntPtr)outSize,
                input,  (UIntPtr)input.Length,
                (UIntPtr)navg, (UIntPtr)nfft,
                (int)window, (int)scale));
            return output;
        }

        // ---------------------------------------------------------------
        // Real FFT  –  quantized input
        // ---------------------------------------------------------------

        /// <summary>
        /// Computes the real FFT of a 16-bit quantized input array.
        /// </summary>
        public static double[] Rfft(short[] input,
            int n, int navg, int nfft,
            Window window = Window.NoWindow,
            CodeFormat format = CodeFormat.TwosComplement,
            RfftScale scale = RfftScale.DbfsSin)
        {
            int outSize = RfftSize(input.Length, navg, nfft);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_rfft16(
                output, (UIntPtr)outSize,
                input,  (UIntPtr)input.Length,
                n, (UIntPtr)navg, (UIntPtr)nfft,
                (int)window, (int)format, (int)scale));
            return output;
        }

        /// <summary>
        /// Computes the real FFT of a 32-bit quantized input array.
        /// </summary>
        public static double[] Rfft(int[] input,
            int n, int navg, int nfft,
            Window window = Window.NoWindow,
            CodeFormat format = CodeFormat.TwosComplement,
            RfftScale scale = RfftScale.DbfsSin)
        {
            int outSize = RfftSize(input.Length, navg, nfft);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_rfft32(
                output, (UIntPtr)outSize,
                input,  (UIntPtr)input.Length,
                n, (UIntPtr)navg, (UIntPtr)nfft,
                (int)window, (int)format, (int)scale));
            return output;
        }

        /// <summary>
        /// Computes the real FFT of a 64-bit quantized input array.
        /// </summary>
        public static double[] Rfft(long[] input,
            int n, int navg, int nfft,
            Window window = Window.NoWindow,
            CodeFormat format = CodeFormat.TwosComplement,
            RfftScale scale = RfftScale.DbfsSin)
        {
            int outSize = RfftSize(input.Length, navg, nfft);
            var output  = new double[outSize];
            Util.Check(NativeMethods.gn_rfft64(
                output, (UIntPtr)outSize,
                input,  (UIntPtr)input.Length,
                n, (UIntPtr)navg, (UIntPtr)nfft,
                (int)window, (int)format, (int)scale));
            return output;
        }
    }
}
