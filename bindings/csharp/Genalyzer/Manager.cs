// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;
using System.Text;

namespace Genalyzer
{
    /// <summary>
    /// Object manager: controls the lifetime of all named objects created
    /// in the library (e.g. Fourier analysis configurations).
    /// </summary>
    public static class Manager
    {
        /// <summary>Removes all objects from the manager.</summary>
        public static void Clear()
            => NativeMethods.gn_mgr_clear();

        /// <summary>
        /// Returns true when both objects are structurally equal.
        /// </summary>
        public static bool Compare(string objKey1, string objKey2)
        {
            Util.Check(NativeMethods.gn_mgr_compare(
                out bool result, objKey1, objKey2));
            return result;
        }

        /// <summary>Returns true when the manager contains the given key.</summary>
        public static bool Contains(string objKey)
        {
            NativeMethods.gn_mgr_contains(out bool result, objKey);
            return result;
        }

        /// <summary>Removes the object with the given key.</summary>
        public static void Remove(string objKey)
            => NativeMethods.gn_mgr_remove(objKey);

        /// <summary>
        /// Saves the named object to a JSON file.
        /// Returns the actual filename used.
        /// </summary>
        public static string Save(string objKey, string filename = "")
        {
            Util.Check(NativeMethods.gn_mgr_save_filename_size(
                out UIntPtr sz, objKey, filename));
            var buf = new byte[(int)sz];
            Util.Check(NativeMethods.gn_mgr_save(
                buf, sz, objKey, filename));
            return Util.BytesToString(buf);
        }

        /// <summary>Returns the number of objects in the manager.</summary>
        public static int Size()
        {
            NativeMethods.gn_mgr_size(out UIntPtr sz);
            return (int)sz;
        }

        /// <summary>
        /// Returns a string representation of the named object.
        /// Pass an empty string to get the whole manager state.
        /// </summary>
        public static string ToString(string objKey = "")
        {
            Util.Check(NativeMethods.gn_mgr_to_string_size(
                out UIntPtr sz, objKey));
            var buf = new byte[(int)sz];
            Util.Check(NativeMethods.gn_mgr_to_string(buf, sz, objKey));
            return Util.BytesToString(buf);
        }

        /// <summary>Returns the type name string of the named object.</summary>
        public static string TypeOf(string objKey)
        {
            Util.Check(NativeMethods.gn_mgr_type_size(
                out UIntPtr sz, objKey));
            var buf = new byte[(int)sz];
            Util.Check(NativeMethods.gn_mgr_type(buf, sz, objKey));
            return Util.BytesToString(buf);
        }
    }
}
