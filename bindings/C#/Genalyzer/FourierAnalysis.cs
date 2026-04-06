// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Genalyzer
{
    /// <summary>
    /// Fourier analysis: configuration, execution, and result retrieval.
    /// </summary>
    public static class FourierAnalysis
    {
        // ---------------------------------------------------------------
        // Configuration
        // ---------------------------------------------------------------

        /// <summary>Creates a Fourier analysis object with the given key.</summary>
        public static void Create(string objKey)
            => Util.Check(NativeMethods.gn_fa_create(objKey));

        /// <summary>Resets a Fourier analysis object to its default state.</summary>
        public static void Reset(string objKey)
            => Util.Check(NativeMethods.gn_fa_reset(objKey));

        /// <summary>Sets the analysis band (numeric overload).</summary>
        public static void SetAnalysisBand(string objKey, double center, double width)
            => Util.Check(NativeMethods.gn_fa_analysis_band(objKey, center, width));

        /// <summary>Sets the analysis band (expression overload).</summary>
        public static void SetAnalysisBandExpr(string objKey,
            string centerExpr, string widthExpr)
            => Util.Check(NativeMethods.gn_fa_analysis_band_e(
                objKey, centerExpr, widthExpr));

        /// <summary>Sets the clock divisors array.</summary>
        public static void SetClk(string objKey, int[] clk, bool asNoise = false)
            => Util.Check(NativeMethods.gn_fa_clk(
                objKey, clk, (UIntPtr)clk.Length, asNoise));

        /// <summary>Enables or disables converter offset.</summary>
        public static void SetConvOffset(string objKey, bool enable)
            => Util.Check(NativeMethods.gn_fa_conv_offset(objKey, enable));

        /// <summary>Treats the DC component as distortion when true.</summary>
        public static void SetDcAsDist(string objKey, bool asDist)
            => Util.Check(NativeMethods.gn_fa_dc(objKey, asDist));

        /// <summary>Sets the data rate (numeric).</summary>
        public static void SetFdata(string objKey, double f)
            => Util.Check(NativeMethods.gn_fa_fdata(objKey, f));

        /// <summary>Sets the data rate (expression).</summary>
        public static void SetFdataExpr(string objKey, string expr)
            => Util.Check(NativeMethods.gn_fa_fdata_e(objKey, expr));

        /// <summary>Adds a fixed-frequency tone component (numeric).</summary>
        public static void AddFixedTone(string objKey, string compKey,
            FACompTag tag, double freq, int ssb = -1)
            => Util.Check(NativeMethods.gn_fa_fixed_tone(
                objKey, compKey, (int)tag, freq, ssb));

        /// <summary>Adds a fixed-frequency tone component (expression).</summary>
        public static void AddFixedToneExpr(string objKey, string compKey,
            FACompTag tag, string freqExpr, int ssb = -1)
            => Util.Check(NativeMethods.gn_fa_fixed_tone_e(
                objKey, compKey, (int)tag, freqExpr, ssb));

        /// <summary>Sets the sample rate (numeric).</summary>
        public static void SetFsample(string objKey, double f)
            => Util.Check(NativeMethods.gn_fa_fsample(objKey, f));

        /// <summary>Sets the sample rate (expression).</summary>
        public static void SetFsampleExpr(string objKey, string expr)
            => Util.Check(NativeMethods.gn_fa_fsample_e(objKey, expr));

        /// <summary>Sets the shift frequency (numeric).</summary>
        public static void SetFshift(string objKey, double f)
            => Util.Check(NativeMethods.gn_fa_fshift(objKey, f));

        /// <summary>Sets the shift frequency (expression).</summary>
        public static void SetFshiftExpr(string objKey, string expr)
            => Util.Check(NativeMethods.gn_fa_fshift_e(objKey, expr));

        /// <summary>Enables or disables fundamental images.</summary>
        public static void SetFundImages(string objKey, bool enable)
            => Util.Check(NativeMethods.gn_fa_fund_images(objKey, enable));

        /// <summary>Sets the maximum harmonic distortion order.</summary>
        public static void SetHd(string objKey, int n)
            => Util.Check(NativeMethods.gn_fa_hd(objKey, n));

        /// <summary>Sets the interleaving factor array.</summary>
        public static void SetIlv(string objKey, int[] ilv, bool asNoise = false)
            => Util.Check(NativeMethods.gn_fa_ilv(
                objKey, ilv, (UIntPtr)ilv.Length, asNoise));

        /// <summary>Sets the intermodulation distortion order.</summary>
        public static void SetImd(string objKey, int n)
            => Util.Check(NativeMethods.gn_fa_imd(objKey, n));

        /// <summary>Adds a maximum-search tone component.</summary>
        public static void AddMaxTone(string objKey, string compKey,
            FACompTag tag, int ssb = -1)
            => Util.Check(NativeMethods.gn_fa_max_tone(
                objKey, compKey, (int)tag, ssb));

        /// <summary>Enables or disables quadrature-error components.</summary>
        public static void SetQuadErrors(string objKey, bool enable)
            => Util.Check(NativeMethods.gn_fa_quad_errors(objKey, enable));

        /// <summary>Removes a component from the analysis object.</summary>
        public static void RemoveComp(string objKey, string compKey)
            => Util.Check(NativeMethods.gn_fa_remove_comp(objKey, compKey));

        /// <summary>
        /// Sets the number of single-side bins for a component group.
        /// </summary>
        public static void SetSsb(string objKey, FASsb group, int ssb)
            => Util.Check(NativeMethods.gn_fa_ssb(objKey, (int)group, ssb));

        /// <summary>Sets a named variable in the analysis object.</summary>
        public static void SetVar(string objKey, string name, double value)
            => Util.Check(NativeMethods.gn_fa_var(objKey, name, value));

        /// <summary>Sets the number of WorstOther components.</summary>
        public static void SetWo(string objKey, int n)
            => Util.Check(NativeMethods.gn_fa_wo(objKey, n));

        // ---------------------------------------------------------------
        // Load / Save / Preview
        // ---------------------------------------------------------------

        /// <summary>
        /// Loads a Fourier analysis configuration from a JSON file.
        /// Returns the object key that was used.
        /// </summary>
        public static string Load(string filename, string objKey = "")
        {
            Util.Check(NativeMethods.gn_fa_load_key_size(
                out UIntPtr sz, filename, objKey));
            var buf = new byte[(int)sz];
            Util.Check(NativeMethods.gn_fa_load(buf, sz, filename, objKey));
            return Util.BytesToString(buf);
        }

        /// <summary>
        /// Returns a human-readable preview of the Fourier analysis
        /// component list.
        /// </summary>
        public static string Preview(string cfgId, bool complex = false)
        {
            Util.Check(NativeMethods.gn_fa_preview_size(
                out UIntPtr sz, cfgId, complex));
            var buf = new byte[(int)sz];
            Util.Check(NativeMethods.gn_fa_preview(buf, sz, cfgId, complex));
            return Util.BytesToString(buf);
        }

        // ---------------------------------------------------------------
        // Analysis execution
        // ---------------------------------------------------------------

        /// <summary>
        /// Executes Fourier analysis and returns all results as a dictionary.
        /// <paramref name="fftData"/> is an interleaved Re/Im double array.
        /// </summary>
        public static Dictionary<string, double> Analyze(
            string cfgId, double[] fftData, int nfft,
            FreqAxisType axisType = FreqAxisType.DcLeft)
        {
            Util.Check(NativeMethods.gn_fft_analysis_results_size(
                out UIntPtr sz, cfgId,
                (UIntPtr)fftData.Length, (UIntPtr)nfft));
            int n = (int)sz;

            var keySizes = new UIntPtr[n];
            Util.Check(NativeMethods.gn_fft_analysis_results_key_sizes(
                keySizes, (UIntPtr)n, cfgId,
                (UIntPtr)fftData.Length, (UIntPtr)nfft));

            var (handles, pins) = Util.AllocKeyBuffers(keySizes);
            var values = new double[n];
            try
            {
                Util.Check(NativeMethods.gn_fft_analysis(
                    handles, (UIntPtr)n,
                    values,  (UIntPtr)n,
                    cfgId, fftData, (UIntPtr)fftData.Length,
                    (UIntPtr)nfft, (int)axisType));
                string[] keys = Util.KeysToStrings(handles, n);
                return Util.MakeResultDictionary(keys, values);
            }
            finally
            {
                Util.FreeKeyBuffers(pins);
            }
        }

        /// <summary>
        /// Executes Fourier analysis and returns only the requested result
        /// keys.
        /// </summary>
        public static double[] AnalyzeSelect(
            string cfgId, double[] fftData, int nfft,
            string[] requestedKeys,
            FreqAxisType axisType = FreqAxisType.DcLeft)
        {
            int n = requestedKeys.Length;
            // build pinned key pointers
            var pins    = new GCHandle[n];
            var ptrKeys = new IntPtr[n];
            for (int i = 0; i < n; i++)
            {
                byte[] b = Encoding.UTF8.GetBytes(requestedKeys[i] + '\0');
                pins[i]    = GCHandle.Alloc(b, GCHandleType.Pinned);
                ptrKeys[i] = pins[i].AddrOfPinnedObject();
            }
            var values = new double[n];
            try
            {
                Util.Check(NativeMethods.gn_fft_analysis_select(
                    values, (UIntPtr)n,
                    cfgId, ptrKeys, (UIntPtr)n,
                    fftData, (UIntPtr)fftData.Length,
                    (UIntPtr)nfft, (int)axisType));
            }
            finally
            {
                Util.FreeKeyBuffers(pins);
            }
            return values;
        }

        /// <summary>
        /// Executes Fourier analysis and returns a single named result.
        /// </summary>
        public static double AnalyzeSingle(
            string cfgId, double[] fftData, int nfft,
            string resultKey,
            FreqAxisType axisType = FreqAxisType.DcLeft)
        {
            Util.Check(NativeMethods.gn_fft_analysis_single(
                out double rvalue,
                cfgId, resultKey,
                fftData, (UIntPtr)fftData.Length,
                (UIntPtr)nfft, (int)axisType));
            return rvalue;
        }

        // ---------------------------------------------------------------
        // Result helpers
        // ---------------------------------------------------------------

        /// <summary>
        /// Looks up a numeric result from a previously computed result
        /// dictionary.
        /// </summary>
        public static double GetResult(
            Dictionary<string, double> results, string key)
        {
            var (ptrKeys, pins, values) =
                Util.DictionaryToKeyValueArrays(results);
            try
            {
                Util.Check(NativeMethods.gn_fa_result(
                    out double result,
                    ptrKeys, (UIntPtr)ptrKeys.Length,
                    values,  (UIntPtr)values.Length,
                    key));
                return result;
            }
            finally
            {
                Util.FreeKeyBuffers(pins);
            }
        }

        /// <summary>
        /// Returns the string representation of a result (e.g. carrier tone
        /// key).
        /// </summary>
        public static string GetResultString(
            Dictionary<string, double> results, string key)
        {
            var (ptrKeys, pins, values) =
                Util.DictionaryToKeyValueArrays(results);
            try
            {
                Util.Check(NativeMethods.gn_fa_result_string_size(
                    out UIntPtr sz,
                    ptrKeys, (UIntPtr)ptrKeys.Length,
                    values,  (UIntPtr)values.Length,
                    key));
                var buf = new byte[(int)sz];
                Util.Check(NativeMethods.gn_fa_result_string(
                    buf, sz,
                    ptrKeys, (UIntPtr)ptrKeys.Length,
                    values,  (UIntPtr)values.Length,
                    key));
                return Util.BytesToString(buf);
            }
            finally
            {
                Util.FreeKeyBuffers(pins);
            }
        }
    }
}
