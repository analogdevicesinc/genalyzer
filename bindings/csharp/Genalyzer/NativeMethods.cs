// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Runtime.InteropServices;

namespace Genalyzer
{
    /// <summary>
    /// Raw P/Invoke declarations for every exported symbol in libgenalyzer.
    /// All signatures match the C API in cgenalyzer.h and
    /// cgenalyzer_simplified_beta.h exactly.
    /// Return value: 0 = success, non-zero = failure (unless noted otherwise).
    /// </summary>
    internal static class NativeMethods
    {
        // ---------------------------------------------------------------
        // Library name - resolved at runtime on Windows / Linux / macOS.
        // ---------------------------------------------------------------
        private const string LibName = "libgenalyzer";

#if NET6_0_OR_GREATER
        // ---------------------------------------------------------------
        // Native-library resolution
        //
        // The [DllImport] declarations above reference the logical name
        // "libgenalyzer".  Default runtime probing often fails to locate the
        // shared library in test/CI layouts and reports an opaque
        // DllNotFoundException.  This module initializer registers an explicit
        // resolver that probes, in order:
        //   1. the GENALYZER_LIB_PATH environment variable (a file or a
        //      directory containing the library),
        //   2. the application base directory,
        //   3. the platform-decorated names (libgenalyzer.so / .dylib /
        //      genalyzer.dll) via the default loader,
        // and otherwise throws a single, actionable error.
        //
        // Guarded to net6.0+; on the net4.7 target System.Runtime.InteropServices
        // .NativeLibrary is unavailable, so the library must be discoverable via
        // the OS loader path (PATH on Windows, LD_LIBRARY_PATH on Linux) or be
        // placed next to the assembly.  See the binding README.
        // ---------------------------------------------------------------
        // CA2255: a module initializer is exactly the right place to register a
        // DllImportResolver - it must run before any P/Invoke is resolved, and
        // this assembly is the sole owner of the "libgenalyzer" import name.
#pragma warning disable CA2255
        [System.Runtime.CompilerServices.ModuleInitializer]
        internal static void RegisterResolver()
        {
            NativeLibrary.SetDllImportResolver(
                typeof(NativeMethods).Assembly, ResolveLibrary);
        }
#pragma warning restore CA2255

        private static IntPtr ResolveLibrary(
            string libraryName, System.Reflection.Assembly assembly,
            DllImportSearchPath? searchPath)
        {
            if (libraryName != LibName)
                return IntPtr.Zero; // not ours; let the default loader handle it

            foreach (var candidate in CandidatePaths())
            {
                if (NativeLibrary.TryLoad(candidate, out IntPtr handle))
                    return handle;
            }

            throw new DllNotFoundException(
                $"Unable to locate the native '{LibName}' library. Set the " +
                "GENALYZER_LIB_PATH environment variable to the library file or " +
                "its containing directory, place it next to the managed " +
                "assembly, or install it on the system loader path. Tried: " +
                string.Join(", ", CandidatePaths()));
        }

        private static System.Collections.Generic.IEnumerable<string> CandidatePaths()
        {
            string[] fileNames = NativeLibraryFileNames();

            string? envPath =
                Environment.GetEnvironmentVariable("GENALYZER_LIB_PATH");
            if (!string.IsNullOrEmpty(envPath))
            {
                // Treat as either a direct file path or a directory.
                if (System.IO.Directory.Exists(envPath))
                    foreach (string name in fileNames)
                        yield return System.IO.Path.Combine(envPath, name);
                else
                    yield return envPath;
            }

            string baseDir = AppContext.BaseDirectory;
            if (!string.IsNullOrEmpty(baseDir))
                foreach (string name in fileNames)
                    yield return System.IO.Path.Combine(baseDir, name);

            // Finally, the bare decorated and logical names so the default OS
            // loader search path (PATH / LD_LIBRARY_PATH / DYLD_LIBRARY_PATH)
            // still applies.
            foreach (string name in fileNames)
                yield return name;
            yield return LibName;
        }

        private static string[] NativeLibraryFileNames()
        {
            // The project builds the native library with OUTPUT_NAME
            // "libgenalyzer" on every platform (see bindings/c/src/CMakeLists.txt),
            // so the decorated name keeps the "lib" prefix even on Windows
            // (libgenalyzer.dll, not genalyzer.dll). The unprefixed name is kept
            // as a secondary candidate for third-party/redistributed builds.
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                return new[] { "libgenalyzer.dll", "genalyzer.dll" };
            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                return new[] { "libgenalyzer.dylib" };
            return new[] { "libgenalyzer.so" };
        }
#endif

        // ===============================================================
        // API Utilities
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_analysis_results_key_sizes(
            [Out] UIntPtr[] keySizes,
            UIntPtr keySizesSize,
            int type);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_analysis_results_size(
            out UIntPtr size,
            int type);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_enum_value(
            out int value,
            [MarshalAs(UnmanagedType.LPStr)] string enumeration,
            [MarshalAs(UnmanagedType.LPStr)] string enumerator);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_error_check(
            [MarshalAs(UnmanagedType.I1)] out bool error);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_error_clear();

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_error_string(
            [Out] byte[] buf,
            UIntPtr size);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_error_string_size(
            out UIntPtr size);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_set_string_termination(
            [MarshalAs(UnmanagedType.I1)] bool nullTerminated);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_version_string(
            [Out] byte[] buf,
            UIntPtr size);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_version_string_size(
            out UIntPtr size);

        // ===============================================================
        // Array Operations
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_abs(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_angle(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_db(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_db10(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_db20(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_norm(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize);

        // ===============================================================
        // Code Density
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_code_axis(
            [Out] double[] output, UIntPtr size,
            int n, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_code_axisx(
            [Out] double[] output, UIntPtr size,
            long min, long max);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_code_density_size(
            out UIntPtr size, int n, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_code_densityx_size(
            out UIntPtr size, long min, long max);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_dnl(
            [Out] double[]  dnl,  UIntPtr dnlSize,
            [In]  ulong[]   hist, UIntPtr histSize,
            int type);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_dnl_analysis(
            [In, Out] IntPtr[] rkeys, UIntPtr rkeysSize,
            [Out]     double[] rvalues, UIntPtr rvaluesSize,
            [In]      double[] dnl,  UIntPtr dnlSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_hist16(
            [Out] ulong[]  hist, UIntPtr histSize,
            [In]  short[]  input, UIntPtr inSize,
            int n, int format,
            [MarshalAs(UnmanagedType.I1)] bool preserve);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_hist32(
            [Out] ulong[]  hist, UIntPtr histSize,
            [In]  int[]    input, UIntPtr inSize,
            int n, int format,
            [MarshalAs(UnmanagedType.I1)] bool preserve);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_hist64(
            [Out] ulong[]  hist, UIntPtr histSize,
            [In]  long[]   input, UIntPtr inSize,
            int n, int format,
            [MarshalAs(UnmanagedType.I1)] bool preserve);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_histx16(
            [Out] ulong[]  hist, UIntPtr histSize,
            [In]  short[]  input, UIntPtr inSize,
            long min, long max,
            [MarshalAs(UnmanagedType.I1)] bool preserve);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_histx32(
            [Out] ulong[]  hist, UIntPtr histSize,
            [In]  int[]    input, UIntPtr inSize,
            long min, long max,
            [MarshalAs(UnmanagedType.I1)] bool preserve);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_histx64(
            [Out] ulong[]  hist, UIntPtr histSize,
            [In]  long[]   input, UIntPtr inSize,
            long min, long max,
            [MarshalAs(UnmanagedType.I1)] bool preserve);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_hist_analysis(
            [In, Out] IntPtr[] rkeys, UIntPtr rkeysSize,
            [Out]     double[] rvalues, UIntPtr rvaluesSize,
            [In]      ulong[]  hist, UIntPtr histSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_inl(
            [Out] double[] inl,  UIntPtr inlSize,
            [In]  double[] dnl,  UIntPtr dnlSize,
            int fit);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_inl_analysis(
            [In, Out] IntPtr[] rkeys, UIntPtr rkeysSize,
            [Out]     double[] rvalues, UIntPtr rvaluesSize,
            [In]      double[] inl,  UIntPtr inlSize);

        // ===============================================================
        // Fourier Analysis
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft_analysis(
            [In, Out] IntPtr[] rkeys,   UIntPtr rkeysSize,
            [Out]     double[] rvalues, UIntPtr rvaluesSize,
            [MarshalAs(UnmanagedType.LPStr)] string cfgId,
            [In]      double[] input,   UIntPtr inSize,
            UIntPtr nfft,
            int axisType);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft_analysis_select(
            [Out] double[] rvalues, UIntPtr rvaluesSize,
            [MarshalAs(UnmanagedType.LPStr)] string cfgId,
            [In]  IntPtr[] rkeys,   UIntPtr rkeysSize,
            [In]  double[] input,   UIntPtr inSize,
            UIntPtr nfft,
            int axisType);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft_analysis_single(
            out double rvalue,
            [MarshalAs(UnmanagedType.LPStr)] string cfgId,
            [MarshalAs(UnmanagedType.LPStr)] string rkey,
            [In] double[] input, UIntPtr inSize,
            UIntPtr nfft,
            int axisType);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft_analysis_results_key_sizes(
            [Out] UIntPtr[] keySizes, UIntPtr keySizesSize,
            [MarshalAs(UnmanagedType.LPStr)] string cfgId,
            UIntPtr inSize,
            UIntPtr nfft);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft_analysis_results_size(
            out UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string cfgId,
            UIntPtr inSize,
            UIntPtr nfft);

        // Fourier Analysis Configuration
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_analysis_band(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            double center, double width);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_analysis_band_e(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string center,
            [MarshalAs(UnmanagedType.LPStr)] string width);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_clk(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [In] int[] clk, UIntPtr clkSize,
            [MarshalAs(UnmanagedType.I1)] bool asNoise);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_conv_offset(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.I1)] bool enable);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_create(
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_dc(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.I1)] bool asDist);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fdata(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            double f);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fdata_e(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string f);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fixed_tone(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string compKey,
            int tag, double freq, int ssb);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fixed_tone_e(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string compKey,
            int tag,
            [MarshalAs(UnmanagedType.LPStr)] string freq,
            int ssb);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fsample(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            double f);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fsample_e(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string f);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fshift(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            double f);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fshift_e(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string f);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_fund_images(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.I1)] bool enable);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_hd(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            int n);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_ilv(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [In] int[] ilv, UIntPtr ilvSize,
            [MarshalAs(UnmanagedType.I1)] bool asNoise);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_imd(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            int n);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_load(
            [Out] byte[] buf, UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string filename,
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_load_key_size(
            out UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string filename,
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_max_tone(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string compKey,
            int tag, int ssb);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_preview(
            [Out] byte[] buf, UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string cfgId,
            [MarshalAs(UnmanagedType.I1)] bool cplx);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_preview_size(
            out UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string cfgId,
            [MarshalAs(UnmanagedType.I1)] bool cplx);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_quad_errors(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.I1)] bool enable);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_remove_comp(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string compKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_reset(
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_result(
            out double result,
            [In]  IntPtr[] rkeys,   UIntPtr rkeysSize,
            [In]  double[] rvalues, UIntPtr rvaluesSize,
            [MarshalAs(UnmanagedType.LPStr)] string rkey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_result_string(
            [Out] byte[] result, UIntPtr resultSize,
            [In]  IntPtr[] rkeys,   UIntPtr rkeysSize,
            [In]  double[] rvalues, UIntPtr rvaluesSize,
            [MarshalAs(UnmanagedType.LPStr)] string rkey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_result_string_size(
            out UIntPtr size,
            [In] IntPtr[] rkeys,   UIntPtr rkeysSize,
            [In] double[] rvalues, UIntPtr rvaluesSize,
            [MarshalAs(UnmanagedType.LPStr)] string rkey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_ssb(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            int group, int ssb);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_var(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string name,
            double value);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fa_wo(
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            int n);

        // ===============================================================
        // Fourier Transforms
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] i,      UIntPtr iSize,
            [In]  double[] q,      UIntPtr qSize,
            UIntPtr navg, UIntPtr nfft, int window);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft16(
            [Out] double[] output, UIntPtr outSize,
            [In]  short[]  i,      UIntPtr iSize,
            [In]  short[]  q,      UIntPtr qSize,
            int n, UIntPtr navg, UIntPtr nfft, int window, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft32(
            [Out] double[] output, UIntPtr outSize,
            [In]  int[]    i,      UIntPtr iSize,
            [In]  int[]    q,      UIntPtr qSize,
            int n, UIntPtr navg, UIntPtr nfft, int window, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft64(
            [Out] double[] output, UIntPtr outSize,
            [In]  long[]   i,      UIntPtr iSize,
            [In]  long[]   q,      UIntPtr qSize,
            int n, UIntPtr navg, UIntPtr nfft, int window, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fft_size(
            out UIntPtr outSize,
            UIntPtr iSize, UIntPtr qSize,
            UIntPtr navg, UIntPtr nfft);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_rfft(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize,
            UIntPtr navg, UIntPtr nfft, int window, int scale);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_rfft16(
            [Out] double[] output, UIntPtr outSize,
            [In]  short[]  input,  UIntPtr inSize,
            int n, UIntPtr navg, UIntPtr nfft, int window, int format, int scale);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_rfft32(
            [Out] double[] output, UIntPtr outSize,
            [In]  int[]    input,  UIntPtr inSize,
            int n, UIntPtr navg, UIntPtr nfft, int window, int format, int scale);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_rfft64(
            [Out] double[] output, UIntPtr outSize,
            [In]  long[]   input,  UIntPtr inSize,
            int n, UIntPtr navg, UIntPtr nfft, int window, int format, int scale);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_rfft_size(
            out UIntPtr outSize,
            UIntPtr inSize, UIntPtr navg, UIntPtr nfft);

        // ===============================================================
        // Fourier Utilities
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_alias(
            out double output,
            double fs, double freq, int axisType);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_coherent(
            out double output,
            UIntPtr nfft, double fs, double freq);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fftshift(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_freq_axis(
            [Out] double[] output, UIntPtr size,
            UIntPtr nfft, int axisType,
            double fs, int axisFormat);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_freq_axis_size(
            out UIntPtr size,
            UIntPtr nfft, int axisType);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_ifftshift(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize);

        // ===============================================================
        // Manager
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_clear();

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_compare(
            [MarshalAs(UnmanagedType.I1)] out bool result,
            [MarshalAs(UnmanagedType.LPStr)] string objKey1,
            [MarshalAs(UnmanagedType.LPStr)] string objKey2);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_contains(
            [MarshalAs(UnmanagedType.I1)] out bool result,
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_remove(
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_save(
            [Out] byte[] buf, UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string filename);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_save_filename_size(
            out UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string objKey,
            [MarshalAs(UnmanagedType.LPStr)] string filename);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_size(
            out UIntPtr size);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_to_string(
            [Out] byte[] buf, UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_to_string_size(
            out UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_type(
            [Out] byte[] buf, UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_mgr_type_size(
            out UIntPtr size,
            [MarshalAs(UnmanagedType.LPStr)] string objKey);

        // ===============================================================
        // Signal Processing
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_downsample(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize,
            int ratio,
            [MarshalAs(UnmanagedType.I1)] bool interleaved);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_downsample16(
            [Out] short[]  output, UIntPtr outSize,
            [In]  short[]  input,  UIntPtr inSize,
            int ratio,
            [MarshalAs(UnmanagedType.I1)] bool interleaved);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_downsample32(
            [Out] int[]    output, UIntPtr outSize,
            [In]  int[]    input,  UIntPtr inSize,
            int ratio,
            [MarshalAs(UnmanagedType.I1)] bool interleaved);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_downsample64(
            [Out] long[]   output, UIntPtr outSize,
            [In]  long[]   input,  UIntPtr inSize,
            int ratio,
            [MarshalAs(UnmanagedType.I1)] bool interleaved);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_downsample_size(
            out UIntPtr outSize,
            UIntPtr inSize, int ratio,
            [MarshalAs(UnmanagedType.I1)] bool interleaved);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fshift(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] i,      UIntPtr iSize,
            [In]  double[] q,      UIntPtr qSize,
            double fs, double fshift);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fshift16(
            [Out] short[]  output, UIntPtr outSize,
            [In]  short[]  i,      UIntPtr iSize,
            [In]  short[]  q,      UIntPtr qSize,
            int n, double fs, double fshift, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fshift32(
            [Out] int[]    output, UIntPtr outSize,
            [In]  int[]    i,      UIntPtr iSize,
            [In]  int[]    q,      UIntPtr qSize,
            int n, double fs, double fshift, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fshift64(
            [Out] long[]   output, UIntPtr outSize,
            [In]  long[]   i,      UIntPtr iSize,
            [In]  long[]   q,      UIntPtr qSize,
            int n, double fs, double fshift, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fshift_size(
            out UIntPtr outSize,
            UIntPtr iSize, UIntPtr qSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_normalize16(
            [Out] double[] output, UIntPtr outSize,
            [In]  short[]  input,  UIntPtr inSize,
            int n, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_normalize32(
            [Out] double[] output, UIntPtr outSize,
            [In]  int[]    input,  UIntPtr inSize,
            int n, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_normalize64(
            [Out] double[] output, UIntPtr outSize,
            [In]  long[]   input,  UIntPtr inSize,
            int n, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_polyval(
            [Out] double[] output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize,
            [In]  double[] c,      UIntPtr cSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_quantize16(
            [Out] short[]  output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize,
            double fsr, int n, double noise, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_quantize32(
            [Out] int[]    output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize,
            double fsr, int n, double noise, int format);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_quantize64(
            [Out] long[]   output, UIntPtr outSize,
            [In]  double[] input,  UIntPtr inSize,
            double fsr, int n, double noise, int format);

        // ===============================================================
        // Waveforms
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_cos(
            [Out] double[] output, UIntPtr size,
            double fs, double ampl, double freq,
            double phase, double td, double tj);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_gaussian(
            [Out] double[] output, UIntPtr size,
            double mean, double sd);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_ramp(
            [Out] double[] output, UIntPtr size,
            double start, double stop, double noise);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_sin(
            [Out] double[] output, UIntPtr size,
            double fs, double ampl, double freq,
            double phase, double td, double tjrms);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_wf_analysis(
            [In, Out] IntPtr[] rkeys,   UIntPtr rkeysSize,
            [Out]     double[] rvalues, UIntPtr rvaluesSize,
            [In]      double[] input,   UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_wf_analysis16(
            [In, Out] IntPtr[] rkeys,   UIntPtr rkeysSize,
            [Out]     double[] rvalues, UIntPtr rvaluesSize,
            [In]      short[]  input,   UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_wf_analysis32(
            [In, Out] IntPtr[] rkeys,   UIntPtr rkeysSize,
            [Out]     double[] rvalues, UIntPtr rvaluesSize,
            [In]      int[]    input,   UIntPtr inSize);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_wf_analysis64(
            [In, Out] IntPtr[] rkeys,   UIntPtr rkeysSize,
            [Out]     double[] rvalues, UIntPtr rvaluesSize,
            [In]      long[]   input,   UIntPtr inSize);

        // ===============================================================
        // Simplified Beta API  (cgenalyzer_simplified_beta.h)
        // ===============================================================

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_free(ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_ttype(int ttype, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_npts(UIntPtr npts, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_get_npts(out UIntPtr npts, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_sample_rate(double sampleRate, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_get_sample_rate(out double sampleRate, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_data_rate(double dataRate, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_shift_freq(double shiftFreq, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_num_tones(UIntPtr numTones, ref IntPtr c);

        // The tone arrays are passed as IntPtr (not double[]) because the
        // native side STORES the pointer in the config struct and dereferences
        // it later (in gn_gen_real_tone / gn_gen_complex_tone).  Default
        // double[] marshaling only pins the array for the duration of this
        // single call, leaving a dangling pointer afterwards.  The managed
        // wrapper pins the arrays in long-lived GCHandles and passes their
        // stable addresses here.
        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_tone_freq(
            IntPtr toneFreq, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_tone_ampl(
            IntPtr toneAmpl, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_tone_phase(
            IntPtr tonePhase, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_fsr(double fsr, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_qres(int qres, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_noise_rms(double noiseRms, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_code_format(int codeFormat, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_nfft(UIntPtr nfft, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_get_nfft(out UIntPtr nfft, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_fft_navg(UIntPtr fftNavg, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_win(int win, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_ssb_fund(int ssbFund, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_ssb_rest(int ssbRest, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_max_harm_order(int maxHarmOrder, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_dnla_signal_type(int dnlaSignalType, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_inla_fit(int inlaFit, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_ramp_start(double rampStart, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_set_ramp_stop(double rampStop, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_get_code_density_size(
            out UIntPtr codeDensitySize, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_gen_tone(
            int ttype, UIntPtr npts, double sampleRate,
            UIntPtr numTones,
            IntPtr toneFreq,
            IntPtr toneAmpl,
            IntPtr tonePhase,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_gen_ramp(
            UIntPtr npts, double rampStart, double rampStop, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_quantize(
            UIntPtr npts, double fsr, int qres, double qnoise, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_histz_nla(
            UIntPtr npts, int qres, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_fftz(
            UIntPtr npts, int qres, UIntPtr navg, UIntPtr nfft,
            int win, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_fa(
            double fixedToneFreq, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_config_fa_auto(
            byte ssbWidth, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_gen_ramp(
            out IntPtr output, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_gen_real_tone(
            out IntPtr output, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_gen_complex_tone(
            out IntPtr outI, out IntPtr outQ, ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_quantize(
            out IntPtr output,
            [In] double[] input,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_fftz(
            out IntPtr output,
            [In] int[] inI,
            [In] int[] inQ,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_histz(
            out IntPtr hist, out UIntPtr histLen,
            [In] int[] qwf,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_dnlz(
            out IntPtr dnl, out UIntPtr dnlLen,
            [In] ulong[] hist,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_inlz(
            out IntPtr inl, out UIntPtr inlLen,
            [In] double[] dnl,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_get_wfa_results(
            out IntPtr rkeys, out IntPtr rvalues,
            out UIntPtr resultsSize,
            [In] int[] qwf,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_get_ha_results(
            out IntPtr rkeys, out IntPtr rvalues,
            out UIntPtr resultsSize,
            [In] ulong[] hist,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_get_dnla_results(
            out IntPtr rkeys, out IntPtr rvalues,
            out UIntPtr resultsSize,
            [In] double[] dnl,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_get_inla_results(
            out IntPtr rkeys, out IntPtr rvalues,
            out UIntPtr resultsSize,
            [In] double[] inl,
            ref IntPtr c);

        [DllImport(LibName,CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_get_fa_single_result(
            out double rvalue,
            [MarshalAs(UnmanagedType.LPStr)] string metricName,
            [In] double[] fftIlv,
            ref IntPtr c);

        [DllImport(LibName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int gn_get_fa_results(
            out IntPtr rkeys, out IntPtr rvalues,
            out UIntPtr resultsSize,
            [In] double[] fftIlv,
            ref IntPtr c);
    }
}
