// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Collections.Generic;

namespace Genalyzer
{
    /// <summary>
    /// Code-density routines: histogram, DNL, INL, and their analysis
    /// counterparts.
    /// </summary>
    public static class CodeDensity
    {
        // ---------------------------------------------------------------
        // Size helpers
        // ---------------------------------------------------------------

        /// <summary>
        /// Returns the array size needed for a code-density operation with
        /// the given resolution and code format.
        /// </summary>
        public static int CodeDensitySize(int n, CodeFormat format)
        {
            Util.Check(NativeMethods.gn_code_density_size(
                out UIntPtr size, n, (int)format));
            return (int)size;
        }

        /// <summary>
        /// Returns the array size needed for a code-density operation over
        /// [min, max].
        /// </summary>
        public static int CodeDensityXSize(long min, long max)
        {
            Util.Check(NativeMethods.gn_code_densityx_size(
                out UIntPtr size, min, max));
            return (int)size;
        }

        // ---------------------------------------------------------------
        // Code axis
        // ---------------------------------------------------------------

        /// <summary>
        /// Fills an array with code-axis values for a given resolution and
        /// code format.
        /// </summary>
        public static double[] CodeAxis(int n,
            CodeFormat format = CodeFormat.TwosComplement)
        {
            int size = CodeDensitySize(n, format);
            var output = new double[size];
            Util.Check(NativeMethods.gn_code_axis(
                output, (UIntPtr)size, n, (int)format));
            return output;
        }

        /// <summary>
        /// Fills an array with code-axis values over the range [min, max].
        /// </summary>
        public static double[] CodeAxisX(long min, long max)
        {
            int size = CodeDensityXSize(min, max);
            var output = new double[size];
            Util.Check(NativeMethods.gn_code_axisx(
                output, (UIntPtr)size, min, max));
            return output;
        }

        // ---------------------------------------------------------------
        // Histogram
        // ---------------------------------------------------------------

        /// <summary>
        /// Computes the histogram of a 16-bit input waveform.
        /// </summary>
        public static ulong[] Hist(short[] input, int n,
            CodeFormat format = CodeFormat.TwosComplement,
            bool preserve = false)
        {
            int size = CodeDensitySize(n, format);
            var hist = new ulong[size];
            Util.Check(NativeMethods.gn_hist16(
                hist, (UIntPtr)size,
                input, (UIntPtr)input.Length,
                n, (int)format, preserve));
            return hist;
        }

        /// <summary>
        /// Computes the histogram of a 32-bit input waveform.
        /// </summary>
        public static ulong[] Hist(int[] input, int n,
            CodeFormat format = CodeFormat.TwosComplement,
            bool preserve = false)
        {
            int size = CodeDensitySize(n, format);
            var hist = new ulong[size];
            Util.Check(NativeMethods.gn_hist32(
                hist, (UIntPtr)size,
                input, (UIntPtr)input.Length,
                n, (int)format, preserve));
            return hist;
        }

        /// <summary>
        /// Computes the histogram of a 64-bit input waveform.
        /// </summary>
        public static ulong[] Hist(long[] input, int n,
            CodeFormat format = CodeFormat.TwosComplement,
            bool preserve = false)
        {
            int size = CodeDensitySize(n, format);
            var hist = new ulong[size];
            Util.Check(NativeMethods.gn_hist64(
                hist, (UIntPtr)size,
                input, (UIntPtr)input.Length,
                n, (int)format, preserve));
            return hist;
        }

        /// <summary>
        /// Computes the histogram of a 16-bit waveform over [min, max].
        /// </summary>
        public static ulong[] HistX(short[] input, long min, long max,
            bool preserve = false)
        {
            int size = CodeDensityXSize(min, max);
            var hist = new ulong[size];
            Util.Check(NativeMethods.gn_histx16(
                hist, (UIntPtr)size,
                input, (UIntPtr)input.Length,
                min, max, preserve));
            return hist;
        }

        /// <summary>
        /// Computes the histogram of a 32-bit waveform over [min, max].
        /// </summary>
        public static ulong[] HistX(int[] input, long min, long max,
            bool preserve = false)
        {
            int size = CodeDensityXSize(min, max);
            var hist = new ulong[size];
            Util.Check(NativeMethods.gn_histx32(
                hist, (UIntPtr)size,
                input, (UIntPtr)input.Length,
                min, max, preserve));
            return hist;
        }

        /// <summary>
        /// Computes the histogram of a 64-bit waveform over [min, max].
        /// </summary>
        public static ulong[] HistX(long[] input, long min, long max,
            bool preserve = false)
        {
            int size = CodeDensityXSize(min, max);
            var hist = new ulong[size];
            Util.Check(NativeMethods.gn_histx64(
                hist, (UIntPtr)size,
                input, (UIntPtr)input.Length,
                min, max, preserve));
            return hist;
        }

        // ---------------------------------------------------------------
        // DNL / INL
        // ---------------------------------------------------------------

        /// <summary>
        /// Computes the DNL from a histogram array.
        /// </summary>
        public static double[] Dnl(ulong[] hist,
            DnlSignal signalType = DnlSignal.Tone)
        {
            var dnl = new double[hist.Length];
            Util.Check(NativeMethods.gn_dnl(
                dnl,  (UIntPtr)dnl.Length,
                hist, (UIntPtr)hist.Length,
                (int)signalType));
            return dnl;
        }

        /// <summary>
        /// Computes the INL from a DNL array.
        /// </summary>
        public static double[] Inl(double[] dnl,
            InlLineFit fit = InlLineFit.BestFit)
        {
            var inl = new double[dnl.Length];
            Util.Check(NativeMethods.gn_inl(
                inl, (UIntPtr)inl.Length,
                dnl, (UIntPtr)dnl.Length,
                (int)fit));
            return inl;
        }

        // ---------------------------------------------------------------
        // Analysis
        // ---------------------------------------------------------------

        /// <summary>
        /// Runs DNL analysis and returns a result dictionary.
        /// Keys: min, max, avg, rms, min_index, max_index,
        ///       first_nm_index, last_nm_index, nm_range.
        /// </summary>
        public static Dictionary<string, double> DnlAnalysis(double[] dnl)
        {
            return RunAnalysis(AnalysisType.DNL,
                (rkeys, rkeysSize, rvalues, rvaluesSize) =>
                    NativeMethods.gn_dnl_analysis(
                        rkeys, rkeysSize, rvalues, rvaluesSize,
                        dnl, (UIntPtr)dnl.Length));
        }

        /// <summary>
        /// Runs histogram analysis and returns a result dictionary.
        /// Keys: sum, first_nz_index, last_nz_index, nz_range.
        /// </summary>
        public static Dictionary<string, double> HistAnalysis(ulong[] hist)
        {
            return RunAnalysis(AnalysisType.Histogram,
                (rkeys, rkeysSize, rvalues, rvaluesSize) =>
                    NativeMethods.gn_hist_analysis(
                        rkeys, rkeysSize, rvalues, rvaluesSize,
                        hist, (UIntPtr)hist.Length));
        }

        /// <summary>
        /// Runs INL analysis and returns a result dictionary.
        /// Keys: min, max, min_index, max_index.
        /// </summary>
        public static Dictionary<string, double> InlAnalysis(double[] inl)
        {
            return RunAnalysis(AnalysisType.INL,
                (rkeys, rkeysSize, rvalues, rvaluesSize) =>
                    NativeMethods.gn_inl_analysis(
                        rkeys, rkeysSize, rvalues, rvaluesSize,
                        inl, (UIntPtr)inl.Length));
        }

        // ---------------------------------------------------------------
        // Private helper
        // ---------------------------------------------------------------

        private delegate int AnalysisFunc(
            System.IntPtr[] rkeys, UIntPtr rkeysSize,
            double[] rvalues, UIntPtr rvaluesSize);

        private static Dictionary<string, double> RunAnalysis(
            AnalysisType type, AnalysisFunc fn)
        {
            int n = ApiUtilities.AnalysisResultsSize(type);
            var keySizes = new UIntPtr[n];
            Util.Check(NativeMethods.gn_analysis_results_key_sizes(
                keySizes, (UIntPtr)n, (int)type));

            var (handles, pins) = Util.AllocKeyBuffers(keySizes);
            var values = new double[n];
            try
            {
                Util.Check(fn(handles, (UIntPtr)n, values, (UIntPtr)n));
                string[] keys = Util.KeysToStrings(handles, n);
                return Util.MakeResultDictionary(keys, values);
            }
            finally
            {
                Util.FreeKeyBuffers(pins);
            }
        }
    }
}
