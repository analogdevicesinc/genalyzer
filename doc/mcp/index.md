# MCP Server

Genalyzer provides a [Model Context Protocol](https://modelcontextprotocol.io/) (MCP) server that exposes spectral analysis tools to AI assistants and other MCP clients. This allows LLM-based workflows to generate test tones, compute FFTs, and analyze RF performance metrics programmatically.

## Installation

Install the optional `mcp` dependency:

```bash
pip install genalyzer[mcp]
```

The MCP server also requires the native `libgenalyzer` library. See [Installation](../setup.md) for build instructions.

## Running the Server

Start the server using the console entry point:

```bash
genalyzer-mcp
```

Or run it directly as a module:

```bash
python -m genalyzer.mcp_server
```

### Claude Desktop Configuration

To use the server with Claude Desktop, add the following to your Claude Desktop configuration:

```json
{
  "mcpServers": {
    "genalyzer": {
      "command": "genalyzer-mcp"
    }
  }
}
```

## Tools

The MCP server exposes four tools for spectral analysis workflows. These tools can be composed together to form a complete analysis pipeline: generate (or import) data, compute the FFT, and extract performance metrics.

```{eval-rst}
.. mermaid::

    graph LR;
        A[generate_test_tone] --> B[compute_fft];
        B --> C[get_fa_metrics];
        A --> D[analyze_spectrum];

        style D fill:#9fa4fc
```

### generate_test_tone

Generate a synthetic complex sinusoidal test tone and save it as a `.npy` file.

**Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `num_points` | int | *required* | Number of samples to generate |
| `sample_rate` | float | *required* | Sample rate in Hz |
| `tone_freq` | float | *required* | Tone frequency in Hz |
| `amplitude` | float | 0.9 | Amplitude (0.0 to 1.0) |
| `output_path` | str | auto | Path to save the `.npy` file |

**Returns:** Dictionary with `output_path`, `num_points`, `sample_rate`, and `tone_freq`.

**Example prompt:**
> Generate a 250 MSPS test tone at 30 MHz with 8192 points.

### compute_fft

Compute the FFT of time-domain data stored in a `.npy` file using the genalyzer native library.

**Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `npy_path` | str | *required* | Path to input `.npy` file |
| `sample_rate` | float | *required* | Sample rate in Hz |
| `nfft` | int | data length | FFT size |
| `output_path` | str | auto | Path to save FFT result `.npy` file |

**Returns:** Dictionary with `output_path`, `nfft`, and `sample_rate`.

Both complex and real input data are supported. Complex data uses `fft()` and real data uses `rfft()`.

### get_fa_metrics

Compute frequency analysis metrics from FFT data. Use this after `compute_fft` when you need fine-grained control over the analysis pipeline.

**Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `fft_npy_path` | str | *required* | Path to `.npy` file containing FFT data |
| `sample_rate` | float | *required* | Sample rate in Hz |
| `tone_freq` | float | *required* | Fundamental tone frequency in Hz |
| `max_harmonics` | int | 6 | Maximum harmonic order to analyze |
| `wo` | int | 0 | Number of worst-other components for SFDR calculation |

**Returns:** Dictionary with `sfdr`, `snr`, `sinad`, `thd`, `enob`, `nsd`, `fbin`, and a full `results` table.

### analyze_spectrum

All-in-one spectral analysis: load time-domain data, compute FFT, extract metrics, and generate an annotated spectrum plot. This is the recommended tool for most use cases.

**Parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `npy_path` | str | *required* | Path to `.npy` file containing time-domain samples |
| `sample_rate` | float | *required* | Sample rate in Hz |
| `nfft` | int | data length | FFT size |
| `num_tones` | int | 1 | Number of tones in the signal |
| `max_harmonics` | int | 6 | Maximum harmonic order to analyze |
| `window` | str | "no_window" | Window function: `"no_window"`, `"blackman_harris"`, or `"hann"` |
| `ssb` | int | 12 | Single-sideband bins for tone grouping. Set to 0 for ideal synthetic data |
| `wo` | int | 0 | Number of worst-other components for SFDR calculation |

**Returns:** Dictionary with `sfdr`, `snr`, `sinad`, `thd`, `enob`, `nsd`, `fbin`, `fft_output_path`, `plot_path`, and a full `results` table.

## Workflow Examples

### Step-by-step analysis

Use the individual tools when you need intermediate results or want to reuse FFT data across multiple analyses:

```python
# 1. Generate a test tone
tone = generate_test_tone(
    num_points=8192,
    sample_rate=250e6,
    tone_freq=30e6,
)

# 2. Compute FFT
fft_result = compute_fft(
    npy_path=tone["output_path"],
    sample_rate=250e6,
)

# 3. Extract metrics
metrics = get_fa_metrics(
    fft_npy_path=fft_result["output_path"],
    sample_rate=250e6,
    tone_freq=30e6,
)
```

### One-shot analysis

Use `analyze_spectrum` to run the entire pipeline in a single call:

```python
result = analyze_spectrum(
    npy_path="/path/to/captured_data.npy",
    sample_rate=250e6,
    window="blackman_harris",
    ssb=12,
)
# result contains: sfdr, snr, thd, enob, plot_path, ...
```

### Analyzing captured hardware data

When working with data captured from a real ADC (e.g., via pyadi-iio), save the samples as a `.npy` file and pass it directly to `analyze_spectrum`:

```python
import numpy as np

# Captured IQ data from hardware
np.save("/tmp/adc_capture.npy", iq_samples)

result = analyze_spectrum(
    npy_path="/tmp/adc_capture.npy",
    sample_rate=250e6,
    window="blackman_harris",
    ssb=12,
)
```

## API Reference

```{eval-rst}

.. automodule:: genalyzer.mcp_server
   :members: generate_test_tone, compute_fft, get_fa_metrics, analyze_spectrum
   :undoc-members:
   :show-inheritance:

```
