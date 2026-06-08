// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

using Genalyzer;
using Xunit;

namespace Genalyzer.Tests
{
    /// <summary>
    /// Initializes the native library once for the whole test run.  Also acts
    /// as a smoke test that the native <c>libgenalyzer</c> is resolvable via the
    /// binding's <c>DllImportResolver</c> (set <c>GENALYZER_LIB_PATH</c> if it
    /// lives outside the default loader path).
    /// </summary>
    public sealed class GenalyzerFixture
    {
        public GenalyzerFixture()
        {
            ApiUtilities.Initialize();
        }
    }

    [CollectionDefinition("Genalyzer")]
    public sealed class GenalyzerCollection : ICollectionFixture<GenalyzerFixture>
    {
    }
}
