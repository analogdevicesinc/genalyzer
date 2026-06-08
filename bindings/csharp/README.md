# Genalyzer C# (.NET) Binding

Managed .NET bindings for [genalyzer](https://github.com/analogdevicesinc/genalyzer),
Analog Devices' C++ library for computing data-converter RF performance metrics.

The binding is a thin, zero-dependency P/Invoke layer over the native C ABI
(`cgenalyzer.h` and `cgenalyzer_simplified_beta.h`). Public types map one-to-one
to the analysis domains:

| Type | Purpose |
|------|---------|
| `Waveforms` | Generate (sin/cos/ramp/gaussian) and analyze time-domain waveforms |
| `FourierTransforms` | FFT / RFFT of real and complex data |
| `FourierAnalysis` | Object-based spectral analysis (configure, analyze, query results) |
| `GenalyzerConfig` | The "simplified-beta" one-object workflow: configure → generate → quantize → FFT → analyze |
| `ArrayOps`, `SignalProcessing`, `CodeDensity` | Array math, quantization, histogram/DNL/INL |
| `Manager` | Lifetime of named native objects |
| `ApiUtilities` | Version, error state, enum lookup, library initialization |

## Requirements

- A **.NET SDK** (6.0 or newer; the library multi-targets `net9.0`, `net6.0`,
  and `net4.7`).
- The native **`libgenalyzer`** shared library
  (`libgenalyzer.so` / `libgenalyzer.dylib` / `genalyzer.dll`) at runtime. Build
  it from the repository root — see the project
  [Installation](../../doc/setup.md) docs:

  ```bash
  cmake -S . -B build -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
  cmake --build build --target genalyzer
  # -> build/bindings/c/src/libgenalyzer.so
  ```

## Building

### With `dotnet`

```bash
cd bindings/csharp
dotnet build Genalyzer/Genalyzer.csproj -c Release   # library, all target frameworks
dotnet pack  Genalyzer/Genalyzer.csproj -c Release    # NuGet package (optional)
```

### With CMake

The binding participates in the root CMake build when enabled. It is **off by
default** and is skipped automatically if no `dotnet` SDK is found, so it never
breaks SDK-less builds:

```bash
cmake -S . -B build -DBUILD_CSHARP_BINDING=ON
cmake --build build --target csharp_binding   # runs `dotnet build`
cmake --build build --target csharp_pack      # runs `dotnet pack` (optional)
```

CMake passes the freshly built native library's directory to the build via
`GENALYZER_LIB_PATH`, so the managed assembly can locate it.

### Windows installer

The Windows installer (built by the `BuildWindowsInstaller` CI job) bundles the
binding alongside the native library: the NuGet package
(`Genalyzer.<version>.nupkg`, containing every target framework) is installed to
`System32`, and the individual managed assemblies are placed under
`System32\Genalyzer\<tfm>\` (`net4.7`, `net6.0`, `net9.0`). Because the native
`libgenalyzer.dll` lands on the loader path, the resolver finds it automatically
for installed-system consumers.

## Native-library discovery

On `net6.0`+ the binding registers a `DllImportResolver` (see
`Genalyzer/NativeMethods.cs`) that locates `libgenalyzer` by probing, in order:

1. **`GENALYZER_LIB_PATH`** — an environment variable pointing either at the
   library file itself or at a directory that contains it;
2. the **application base directory** (next to `Genalyzer.dll`);
3. the platform-decorated name via the **OS loader path**
   (`PATH` / `LD_LIBRARY_PATH` / `DYLD_LIBRARY_PATH`).

If none succeed, a single, actionable `DllNotFoundException` is thrown listing
the paths tried. The typical setup when the library is built in-tree:

```bash
export GENALYZER_LIB_PATH="$PWD/build/bindings/c/src"
```

> **`net4.7` caveat:** .NET Framework lacks `System.Runtime.InteropServices.NativeLibrary`,
> so the `DllImportResolver` is compiled out for that target. On `net4.7` the
> library must be discoverable through the OS loader path (`PATH` on Windows) or
> placed next to the managed assembly — `GENALYZER_LIB_PATH` is **not** honored.

## Usage

```csharp
using System;
using Genalyzer;

// Initialize once (sets null-terminated string mode for marshaling).
ApiUtilities.Initialize();

// Generate and analyze a time-domain waveform.
double[] wave = Waveforms.Sin(nsamples: 8192, fs: 8192.0, ampl: 1.0, freq: 64.0);
var stats = Waveforms.WfAnalysis(wave);
Console.WriteLine($"min={stats["min"]:F3}  max={stats["max"]:F3}  rms={stats["rms"]:F3}");

// Simplified-beta spectral analysis: configure → generate → quantize → FFT → analyze.
using var cfg = new GenalyzerConfig();
cfg.ConfigGenTone(ToneType.ComplexExp, npts: 16384, sampleRate: 3_000_000.0,
    toneFreq: new[] { 300_000.0 }, toneAmpl: new[] { 0.9 }, tonePhase: new[] { 0.0 });
cfg.ConfigQuantize(16384, fsr: 2.0, qres: 12);
cfg.ConfigFftz(16384, 12, navg: 1, nfft: 16384, Window.NoWindow);
cfg.SetSampleRate(3_000_000.0);
cfg.ConfigFa(300_000.0);

var (i, q) = cfg.GenComplexTone();
double[] fft = cfg.Fftz(cfg.Quantize(i), cfg.Quantize(q));
double sfdr = cfg.GetFaSingleResult(fft, "sfdr");
Console.WriteLine($"SFDR = {sfdr:F2} dB");
```

See the runnable [`examples/`](examples) directory for more
(`dotnet run --project examples/FourierAnalysisSimplified`).

## Testing

The test suite (xUnit, under [`tests/`](tests)) targets `net6.0` and `net9.0`:

```bash
cd bindings/csharp
export GENALYZER_LIB_PATH="$PWD/../../build/bindings/c/src"
dotnet test tests/Genalyzer.Tests.csproj
```

The tests are also registered with CTest when the project is configured with
`-DBUILD_CSHARP_BINDING=ON -DBUILD_TESTS_EXAMPLES=ON` (run via
`ctest -R csharp_tests`), and run in CI on Linux and Windows
(`.github/workflows/test-csharp.yml`).

## License

GPL-2.0-or-later (or ADIBSD), matching the rest of genalyzer. See SPDX headers
in each source file.
