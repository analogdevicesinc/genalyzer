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
.. d2::

   direction: right

   G1: generate_test_tone      { class: sw-step-white }
   G2: generate_real_tone      { class: sw-step-white }
   G3: generate_ramp           { class: sw-step-white }
   G4: generate_gaussian_noise { class: sw-step-white }
   Q:  quantize                { class: sw-step-white }
   CF: compute_fft             { class: sw-step-white }
   CH: compute_histogram       { class: sw-step-white }
   CD: compute_dnl             { class: sw-step-white }
   CI: compute_inl             { class: sw-step-white }
   FM: get_fa_metrics          { class: sw-step-white }
   AS: analyze_spectrum        { class: sw-step-blue }
   AH: analyze_histogram       { class: sw-step-blue }
   AD: analyze_dnl             { class: sw-step-blue }
   AI: analyze_inl             { class: sw-step-blue }
   AW: analyze_waveform        { class: sw-step-blue }

   G1 -> Q  { class: sw-flow }
   G2 -> Q  { class: sw-flow }
   G3 -> Q  { class: sw-flow }
   G4 -> Q  { class: sw-flow }
   Q -> CF  { class: sw-flow-data }
   Q -> CH  { class: sw-flow-data }
   CH -> CD { class: sw-flow-data }
   CD -> CI { class: sw-flow-data }
   CF -> FM { class: sw-flow-data }
   Q -> AS  { class: sw-flow-data }
   Q -> AH  { class: sw-flow-data }
   Q -> AD  { class: sw-flow-data }
   Q -> AI  { class: sw-flow-data }
   Q -> AW  { class: sw-flow-data }
```

## See also

```{toctree}
:maxdepth: 1

workflows
reference
```
