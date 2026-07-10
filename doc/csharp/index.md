# C# Bindings

Genalyzer ships managed **.NET (C#) bindings** that wrap the native C ABI with a
thin, zero-dependency P/Invoke layer. The library multi-targets `net9.0`,
`net6.0`, and `net4.7`, and mirrors the same analysis domains exposed by the C
and Python bindings.

The binding sources live in
[`bindings/csharp`](https://github.com/analogdevicesinc/genalyzer/tree/main/bindings/csharp);
see that directory's `README.md` for the full reference.

## Installation

The managed assembly requires the native `libgenalyzer` shared library at
runtime. Build it from the repository root (see [Installation](../setup.md)):

```bash
cmake -S . -B build -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target genalyzer
```

Then build the binding either with the .NET SDK directly:

```bash
cd bindings/csharp
dotnet build Genalyzer/Genalyzer.csproj -c Release
```

or through CMake (off by default; auto-skipped when no `dotnet` SDK is present):

```bash
cmake -S . -B build -DBUILD_CSHARP_BINDING=ON
cmake --build build --target csharp_binding
```

## Locating the native library

On `net6.0`+ the binding installs a `DllImportResolver` that finds
`libgenalyzer` by probing, in order:

1. the **`GENALYZER_LIB_PATH`** environment variable (a library file or a
   directory containing it),
2. the application base directory (next to `Genalyzer.dll`),
3. the OS loader path (`PATH` / `LD_LIBRARY_PATH` / `DYLD_LIBRARY_PATH`).

A failure throws one actionable `DllNotFoundException` listing the paths tried.
For an in-tree build:

```bash
export GENALYZER_LIB_PATH="$PWD/build/bindings/c/src"
```

```{note}
On the `net4.7` target, .NET Framework lacks `NativeLibrary`, so the resolver is
compiled out. There the library must be on the OS loader path or next to the
assembly; `GENALYZER_LIB_PATH` is not honored.
```

## Example

```csharp
using System;
using Genalyzer;

ApiUtilities.Initialize();

// Time-domain generation and analysis.
double[] wave = Waveforms.Sin(8192, fs: 8192.0, ampl: 1.0, freq: 64.0);
var stats = Waveforms.WfAnalysis(wave);
Console.WriteLine($"max={stats["max"]:F3}  rms={stats["rms"]:F3}");

// Simplified-beta spectral analysis.
using var cfg = new GenalyzerConfig();
cfg.ConfigGenTone(ToneType.ComplexExp, 16384, 3_000_000.0,
    new[] { 300_000.0 }, new[] { 0.9 }, new[] { 0.0 });
cfg.ConfigQuantize(16384, 2.0, 12);
cfg.ConfigFftz(16384, 12, 1, 16384, Window.NoWindow);
cfg.SetSampleRate(3_000_000.0);
cfg.ConfigFa(300_000.0);

var (i, q) = cfg.GenComplexTone();
double[] fft = cfg.Fftz(cfg.Quantize(i), cfg.Quantize(q));
Console.WriteLine($"SFDR = {cfg.GetFaSingleResult(fft, "sfdr"):F2} dB");
```

More runnable samples are in
[`bindings/csharp/examples`](https://github.com/analogdevicesinc/genalyzer/tree/main/bindings/csharp/examples).

## Public API surface

| Type | Purpose |
|------|---------|
| `Waveforms` | Generate (`Sin`, `Cos`, `Ramp`, `Gaussian`) and analyze time-domain waveforms |
| `FourierTransforms` | `Fft` / `Rfft` of real and complex data |
| `FourierAnalysis` | Object-based spectral analysis: configure, analyze, query results |
| `GenalyzerConfig` | Simplified-beta one-object configure → generate → quantize → FFT → analyze flow |
| `ArrayOps` | Element-wise array math (abs, angle, dB, norm) |
| `SignalProcessing` | Quantization |
| `CodeDensity` | Histogram, DNL, INL |
| `Manager` | Lifetime of named native objects |
| `ApiUtilities` | Version, error state, enum lookup, initialization |

XML doc-comments are emitted (`GenerateDocumentationFile`) for IDE IntelliSense.

## Testing

The xUnit suite (`bindings/csharp/tests`, targeting `net6.0` and `net9.0`) runs via:

```bash
cd bindings/csharp
export GENALYZER_LIB_PATH="$PWD/../../build/bindings/c/src"
dotnet test tests/Genalyzer.Tests.csproj
```

It is also registered with CTest (`ctest -R csharp_tests`) when configured with
`-DBUILD_CSHARP_BINDING=ON -DBUILD_TESTS_EXAMPLES=ON`, and runs in CI on Linux
and Windows.
