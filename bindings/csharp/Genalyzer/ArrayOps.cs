// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;

namespace Genalyzer
{
    /// <summary>
    /// Array operations: abs, angle, db, db10, db20, norm.
    /// Input arrays contain interleaved Re/Im doubles (complex representation).
    /// </summary>
    public static class ArrayOps
    {
        /// <summary>
        /// Computes the element-wise absolute value of interleaved complex
        /// doubles.
        /// </summary>
        public static double[] Abs(double[] input)
        {
            var output = new double[input.Length / 2];
            Util.Check(NativeMethods.gn_abs(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length));
            return output;
        }

        /// <summary>
        /// Computes the element-wise phase angle of interleaved complex
        /// doubles.
        /// </summary>
        public static double[] Angle(double[] input)
        {
            var output = new double[input.Length / 2];
            Util.Check(NativeMethods.gn_angle(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length));
            return output;
        }

        /// <summary>
        /// Computes 20*log10(|x|) for each complex element in the interleaved
        /// input array.
        /// </summary>
        public static double[] Db(double[] input)
        {
            var output = new double[input.Length / 2];
            Util.Check(NativeMethods.gn_db(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length));
            return output;
        }

        /// <summary>
        /// Computes 10*log10(x) for each element of a real-valued input array.
        /// </summary>
        public static double[] Db10(double[] input)
        {
            var output = new double[input.Length];
            Util.Check(NativeMethods.gn_db10(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length));
            return output;
        }

        /// <summary>
        /// Computes 20*log10(x) for each element of a real-valued input array.
        /// </summary>
        public static double[] Db20(double[] input)
        {
            var output = new double[input.Length];
            Util.Check(NativeMethods.gn_db20(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length));
            return output;
        }

        /// <summary>
        /// Computes the element-wise squared magnitude (norm) of interleaved
        /// complex doubles.
        /// </summary>
        public static double[] Norm(double[] input)
        {
            var output = new double[input.Length / 2];
            Util.Check(NativeMethods.gn_norm(
                output, (UIntPtr)output.Length,
                input,  (UIntPtr)input.Length));
            return output;
        }
    }
}
