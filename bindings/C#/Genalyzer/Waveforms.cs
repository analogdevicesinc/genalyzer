// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Collections.Generic;

namespace Genalyzer
{
    /// <summary>
    /// Waveform generation and waveform analysis.
    /// </summary>
    public static class Waveforms
    {
        // ---------------------------------------------------------------
        // Generation
        // ---------------------------------------------------------------

        /// <summary>
        /// Generates a cosine waveform.
        /// </summary>
        /// <param name="nsamples">Number of samples.</param>
        /// <param name="fs">Sample rate (S/s).</param>
        /// <param name="ampl">Amplitude.</param>
        /// <param name="freq">Frequency (Hz).</param>
        /// <param name="phase">Phase (rad).</param>
        /// <param name="td">Time delay (s).</param>
        /// <param name="tj">RMS aperture jitter (s).</param>
        public static double[] Cos(int nsamples, double fs,
            double ampl, double freq,
            double phase = 0.0, double td = 0.0, double tj = 0.0)
        {
            var output = new double[nsamples];
            Util.Check(NativeMethods.gn_cos(
                output, (UIntPtr)nsamples,
                fs, ampl, freq, phase, td, tj));
            return output;
        }

        /// <summary>
        /// Generates Gaussian random noise samples.
        /// </summary>
        /// <param name="nsamples">Number of samples.</param>
        /// <param name="mean">Mean.</param>
        /// <param name="sd">Standard deviation.</param>
        public static double[] Gaussian(int nsamples, double mean, double sd)
        {
            var output = new double[nsamples];
            Util.Check(NativeMethods.gn_gaussian(
                output, (UIntPtr)nsamples, mean, sd));
            return output;
        }

        /// <summary>
        /// Generates a ramp waveform.
        /// </summary>
        /// <param name="nsamples">Number of samples.</param>
        /// <param name="start">Start value.</param>
        /// <param name="stop">Stop value.</param>
        /// <param name="noise">RMS noise.</param>
        public static double[] Ramp(int nsamples,
            double start, double stop, double noise = 0.0)
        {
            var output = new double[nsamples];
            Util.Check(NativeMethods.gn_ramp(
                output, (UIntPtr)nsamples, start, stop, noise));
            return output;
        }

        /// <summary>
        /// Generates a sine waveform.
        /// </summary>
        /// <param name="nsamples">Number of samples.</param>
        /// <param name="fs">Sample rate (S/s).</param>
        /// <param name="ampl">Amplitude.</param>
        /// <param name="freq">Frequency (Hz).</param>
        /// <param name="phase">Phase (rad).</param>
        /// <param name="td">Time delay (s).</param>
        /// <param name="tjrms">RMS aperture jitter (s).</param>
        public static double[] Sin(int nsamples, double fs,
            double ampl, double freq,
            double phase = 0.0, double td = 0.0, double tjrms = 0.0)
        {
            var output = new double[nsamples];
            Util.Check(NativeMethods.gn_sin(
                output, (UIntPtr)nsamples,
                fs, ampl, freq, phase, td, tjrms));
            return output;
        }

        // ---------------------------------------------------------------
        // Analysis
        // ---------------------------------------------------------------

        /// <summary>
        /// Runs waveform analysis on a normalized double array.
        /// Keys: min, max, mid, range, avg, rms, rmsac, min_index,
        ///       max_index.
        /// </summary>
        public static Dictionary<string, double> WfAnalysis(double[] input)
        {
            return RunWfAnalysis(AnalysisType.Waveform,
                (rkeys, rkeysSize, rvalues, rvaluesSize) =>
                    NativeMethods.gn_wf_analysis(
                        rkeys, rkeysSize, rvalues, rvaluesSize,
                        input, (UIntPtr)input.Length));
        }

        /// <summary>
        /// Runs waveform analysis on a 16-bit integer array.
        /// </summary>
        public static Dictionary<string, double> WfAnalysis(short[] input)
        {
            return RunWfAnalysis(AnalysisType.Waveform,
                (rkeys, rkeysSize, rvalues, rvaluesSize) =>
                    NativeMethods.gn_wf_analysis16(
                        rkeys, rkeysSize, rvalues, rvaluesSize,
                        input, (UIntPtr)input.Length));
        }

        /// <summary>
        /// Runs waveform analysis on a 32-bit integer array.
        /// </summary>
        public static Dictionary<string, double> WfAnalysis(int[] input)
        {
            return RunWfAnalysis(AnalysisType.Waveform,
                (rkeys, rkeysSize, rvalues, rvaluesSize) =>
                    NativeMethods.gn_wf_analysis32(
                        rkeys, rkeysSize, rvalues, rvaluesSize,
                        input, (UIntPtr)input.Length));
        }

        /// <summary>
        /// Runs waveform analysis on a 64-bit integer array.
        /// </summary>
        public static Dictionary<string, double> WfAnalysis(long[] input)
        {
            return RunWfAnalysis(AnalysisType.Waveform,
                (rkeys, rkeysSize, rvalues, rvaluesSize) =>
                    NativeMethods.gn_wf_analysis64(
                        rkeys, rkeysSize, rvalues, rvaluesSize,
                        input, (UIntPtr)input.Length));
        }

        // ---------------------------------------------------------------
        // Private helpers
        // ---------------------------------------------------------------

        private delegate int WfAnalysisFunc(
            System.IntPtr[] rkeys, UIntPtr rkeysSize,
            double[] rvalues, UIntPtr rvaluesSize);

        private static Dictionary<string, double> RunWfAnalysis(
            AnalysisType type, WfAnalysisFunc fn)
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
