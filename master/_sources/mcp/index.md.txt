# MCP Server

Genalyzer provides a [Model Context Protocol](https://modelcontextprotocol.io/) (MCP) server that exposes spectral and code-density analysis tools to AI assistants and other MCP clients. Tools cover all five genalyzer analysis domains — Fourier, histogram, DNL, INL, and time-domain waveform — plus generators and a quantizer for end-to-end simulate-and-verify workflows.

## Installation

```bash
pip install genalyzer[mcp]
```

The MCP server also requires the native `libgenalyzer` library. See [Installation](../setup.md) for build instructions.

## Running the server

```bash
genalyzer-mcp
```

Or as a module:

```bash
python -m genalyzer.mcp.server
```

### Claude Desktop configuration

```json
{
  "mcpServers": {
    "genalyzer": {
      "command": "genalyzer-mcp"
    }
  }
}
```

## Tool inventory

All tools read `.npy` or `.csv` inputs (auto-detected by extension). Every `analyze_*` tool accepts a `plot: bool = False` flag; set to `True` to render an annotated PNG next to the input.

| Tool | Layer | Purpose |
|---|---|---|
| `generate_test_tone` | Generator | Complex sinusoid |
| `generate_real_tone` | Generator | Real sinusoid |
| `generate_ramp` | Generator | Linear ramp |
| `generate_gaussian_noise` | Generator | AWGN |
| `quantize` | Primitive | Apply N-bit quantization |
| `compute_fft` | Primitive | FFT of time-domain data |
| `get_fa_metrics` | Primitive | Metrics from pre-computed FFT |
| `compute_histogram` | Primitive | Code-count histogram |
| `compute_dnl` | Primitive | DNL from histogram |
| `compute_inl` | Primitive | INL from DNL |
| `compute_waveform_stats` | Primitive | Time-domain stats |
| `analyze_spectrum` | All-in-one | Load → FFT → metrics (+ optional PNG) |
| `analyze_histogram` | All-in-one | Load → histogram → metrics (+ optional PNG) |
| `analyze_dnl` | All-in-one | Load → DNL → metrics (+ optional PNG) |
| `analyze_inl` | All-in-one | Load → INL → metrics (+ optional PNG) |
| `analyze_waveform` | All-in-one | Time-domain analysis (+ optional PNG) |

## Pipeline

```{eval-rst}
.. mermaid::

   graph LR;
     G1[generate_test_tone] --> Q[quantize];
     G2[generate_real_tone] --> Q;
     G3[generate_ramp] --> Q;
     G4[generate_gaussian_noise] --> Q;
     Q --> CF[compute_fft];
     Q --> CH[compute_histogram];
     CH --> CD[compute_dnl];
     CD --> CI[compute_inl];
     CF --> FM[get_fa_metrics];
     Q --> AS[analyze_spectrum];
     Q --> AH[analyze_histogram];
     Q --> AD[analyze_dnl];
     Q --> AI[analyze_inl];
     Q --> AW[analyze_waveform];

     style AS fill:#9fa4fc
     style AH fill:#9fa4fc
     style AD fill:#9fa4fc
     style AI fill:#9fa4fc
     style AW fill:#9fa4fc
```

## See also

```{toctree}
:maxdepth: 1

workflows
reference
```
