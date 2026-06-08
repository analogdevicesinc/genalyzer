// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Collections.Generic;
using System.Text;

namespace Genalyzer
{
    /// <summary>
    /// API utilities: version string, error handling, and analysis-results
    /// sizing helpers.
    /// </summary>
    public static class ApiUtilities
    {
        /// <summary>
        /// Initializes the library.  Must be called once before any other
        /// method.  Sets null-terminated string mode so managed marshalling
        /// works correctly.
        /// </summary>
        public static void Initialize()
        {
            Util.Check(NativeMethods.gn_set_string_termination(true));
        }

        /// <summary>Returns the library version string.</summary>
        public static string VersionString()
        {
            Util.Check(NativeMethods.gn_version_string_size(out UIntPtr sz));
            var buf = new byte[(int)sz];
            Util.Check(NativeMethods.gn_version_string(buf, sz));
            return Util.BytesToString(buf);
        }

        /// <summary>Returns true if the library has a pending error.</summary>
        public static bool ErrorCheck()
        {
            NativeMethods.gn_error_check(out bool err);
            return err;
        }

        /// <summary>Clears the library error state.</summary>
        public static void ErrorClear()
            => NativeMethods.gn_error_clear();

        /// <summary>Returns the current library error string (empty when none).</summary>
        public static string ErrorString()
        {
            NativeMethods.gn_error_string_size(out UIntPtr sz);
            var buf = new byte[(int)sz];
            NativeMethods.gn_error_string(buf, sz);
            return Util.BytesToString(buf);
        }

        /// <summary>Returns the underlying integer value of an enumeration member.</summary>
        public static int EnumValue(string enumeration, string enumerator)
        {
            Util.Check(NativeMethods.gn_enum_value(out int v, enumeration, enumerator));
            return v;
        }

        /// <summary>
        /// Returns the number of key-value result pairs for the given
        /// analysis type.
        /// </summary>
        public static int AnalysisResultsSize(AnalysisType type)
        {
            Util.Check(NativeMethods.gn_analysis_results_size(
                out UIntPtr size, (int)type));
            return (int)size;
        }

        /// <summary>
        /// Returns the byte-sizes of each result key for the given analysis
        /// type (including null terminator).
        /// </summary>
        public static int[] AnalysisResultsKeySizes(AnalysisType type)
        {
            int n = AnalysisResultsSize(type);
            var sizes = new UIntPtr[n];
            Util.Check(NativeMethods.gn_analysis_results_key_sizes(
                sizes, (UIntPtr)n, (int)type));
            var result = new int[n];
            for (int i = 0; i < n; i++) result[i] = (int)sizes[i];
            return result;
        }
    }
}
