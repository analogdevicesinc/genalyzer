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
    /// Internal helpers shared across the managed wrapper classes.
    /// Handles string termination, result-key buffer allocation, and
    /// conversion of raw IntPtr key arrays to managed strings.
    /// </summary>
    internal static class Util
    {
        // The library is configured to null-terminate strings once at startup
        // from GenalyzerLibrary.Initialize().

        /// <summary>
        /// Throws <see cref="GenalyzerException"/> if the native call returned
        /// a non-zero value, pulling the error message out of the library.
        /// </summary>
        internal static void Check(int result)
        {
            if (result == 0) return;
            NativeMethods.gn_error_string_size(out UIntPtr sz);
            var buf = new byte[(int)sz];
            NativeMethods.gn_error_string(buf, sz);
            string msg = Encoding.UTF8.GetString(buf).TrimEnd('\0');
            NativeMethods.gn_error_clear();
            throw new GenalyzerException(string.IsNullOrEmpty(msg)
                ? $"Native call failed (code {result})" : msg);
        }

        /// <summary>
        /// Allocates a managed key-buffer array sized by the library, suitable
        /// for passing as the rkeys argument to analysis functions.
        /// </summary>
        internal static (IntPtr[] handles, GCHandle[] pins) AllocKeyBuffers(
            UIntPtr[] keySizes)
        {
            int n = keySizes.Length;
            var handles = new IntPtr[n];
            var pins    = new GCHandle[n];
            for (int i = 0; i < n; i++)
            {
                var bytes = new byte[(int)keySizes[i]];
                pins[i]    = GCHandle.Alloc(bytes, GCHandleType.Pinned);
                handles[i] = pins[i].AddrOfPinnedObject();
            }
            return (handles, pins);
        }

        /// <summary>Releases pinned GC handles allocated by AllocKeyBuffers.</summary>
        internal static void FreeKeyBuffers(GCHandle[] pins)
        {
            foreach (var pin in pins) pin.Free();
        }

        /// <summary>
        /// Converts the raw IntPtr key array written by a native analysis
        /// function into an array of managed strings.
        /// </summary>
        internal static string[] KeysToStrings(IntPtr[] handles, int count)
        {
            var result = new string[count];
            for (int i = 0; i < count; i++)
                result[i] = PtrToStringUtf8(handles[i]);
            return result;
        }

        /// <summary>
        /// Decodes a native, null-terminated UTF-8 C string into a managed
        /// string.  This is the single place native <c>char*</c> strings are
        /// decoded, so the encoding (UTF-8, matching how the library emits and
        /// how <see cref="BytesToString"/> / <see cref="Check"/> already decode)
        /// is defined once.  Implemented without
        /// <c>Marshal.PtrToStringUTF8</c> so it also works on the
        /// <c>net4.7</c> target, where that overload is unavailable.
        /// </summary>
        internal static string PtrToStringUtf8(IntPtr ptr)
        {
            if (ptr == IntPtr.Zero) return string.Empty;
            int len = 0;
            while (Marshal.ReadByte(ptr, len) != 0) len++;
            if (len == 0) return string.Empty;
            var bytes = new byte[len];
            Marshal.Copy(ptr, bytes, 0, len);
            return Encoding.UTF8.GetString(bytes);
        }

        /// <summary>
        /// Builds a Dictionary from parallel key/value arrays returned by
        /// native analysis functions.
        /// </summary>
        internal static Dictionary<string, double> MakeResultDictionary(
            string[] keys, double[] values)
        {
            var dict = new Dictionary<string, double>(keys.Length,
                StringComparer.Ordinal);
            for (int i = 0; i < keys.Length; i++)
                dict[keys[i]] = values[i];
            return dict;
        }

        /// <summary>
        /// Converts a null-terminated byte buffer returned by the library into
        /// a managed string.
        /// </summary>
        internal static string BytesToString(byte[] buf)
            => Encoding.UTF8.GetString(buf).TrimEnd('\0');

        /// <summary>
        /// Builds the parallel IntPtr[] / double[] arrays needed by
        /// gn_fa_result / gn_fa_result_string from a managed dictionary.
        /// The returned handles must remain alive for the duration of the call;
        /// free them with FreeKeyBuffers afterwards.
        /// </summary>
        internal static (IntPtr[] ptrKeys, GCHandle[] pins, double[] values)
            DictionaryToKeyValueArrays(Dictionary<string, double> resultDict)
        {
            int n = resultDict.Count;
            var ptrKeys = new IntPtr[n];
            var pins    = new GCHandle[n];
            var values  = new double[n];
            int i = 0;
            foreach (var kv in resultDict)
            {
                byte[] encoded = Encoding.UTF8.GetBytes(kv.Key + '\0');
                pins[i]    = GCHandle.Alloc(encoded, GCHandleType.Pinned);
                ptrKeys[i] = pins[i].AddrOfPinnedObject();
                values[i]  = kv.Value;
                i++;
            }
            return (ptrKeys, pins, values);
        }
    }
}
