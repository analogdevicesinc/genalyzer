// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using Genalyzer;
using Xunit;

namespace Genalyzer.Tests
{
    [Collection("Genalyzer")]
    public sealed class ApiUtilitiesTests
    {
        [Fact]
        public void VersionString_IsNonEmpty()
        {
            // Exercises the byte-buffer UTF-8 decode path (Util.BytesToString).
            string version = ApiUtilities.VersionString();
            Assert.False(string.IsNullOrWhiteSpace(version));
        }

        [Fact]
        public void EnumValue_ResolvesKnownEnumerator()
        {
            // gn_enum_value round-trips an enumeration/enumerator name to its
            // integer value. The managed AnalysisType enum must agree with the
            // native "AnalysisType" map.
            int value = ApiUtilities.EnumValue("AnalysisType", "Waveform");
            Assert.Equal((int)AnalysisType.Waveform, value);
        }

        [Fact]
        public void AnalysisResultsSize_IsPositiveForWaveform()
        {
            Assert.True(ApiUtilities.AnalysisResultsSize(AnalysisType.Waveform) > 0);
        }
    }
}
