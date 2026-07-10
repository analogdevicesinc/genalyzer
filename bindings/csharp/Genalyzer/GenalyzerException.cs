// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using System;

namespace Genalyzer
{
    /// <summary>
    /// Thrown when a native genalyzer function returns a non-zero error code.
    /// </summary>
    public sealed class GenalyzerException : Exception
    {
        public GenalyzerException(string message) : base(message) { }
        public GenalyzerException(string message, Exception inner)
            : base(message, inner) { }
    }
}
