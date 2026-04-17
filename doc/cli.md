# CLI

Genalyzer provides a `genalyzer` command that mirrors every MCP tool as a grouped subcommand. It is the simpler surface for AI clients and scripted pipelines that prefer plain stdio + JSON over the MCP protocol.

## Installation

```bash
pip install genalyzer[cli]
```

Like the MCP server, the CLI requires the native `libgenalyzer` library. See [Installation](setup.md) for build instructions.

## Running the CLI

```bash
genalyzer --version
genalyzer --help
genalyzer tools        # prints the 16 underlying MCP tool names
```

Every subcommand accepts `--compact` for single-line JSON output (default is pretty-printed) and emits the tool's return dict on stdout.

## Command reference

```{eval-rst}
.. click:: genalyzer.cli.main:cli
   :prog: genalyzer
   :nested: full
```

## Examples

The three workflow patterns documented in [MCP Workflows](mcp/workflows.md) have direct CLI equivalents. Each example uses `jq` to chain output paths between steps.

### Simulate-and-verify a 12-bit converter

```bash
TONE=$(genalyzer generators real-tone \
    --num-points 8192 --sample-rate 250e6 --tone-freq 30e6 --amplitude 0.9 \
    --compact | jq -r .output_path)

Q=$(genalyzer quantize --npy-path "$TONE" --bits 12 --fullscale 2.0 --compact \
    | jq -r .output_path)

genalyzer fourier analyze --npy-path "$Q" --sample-rate 250e6 \
    --window blackman_harris --ssb 3
```

### Analyze a hardware capture

```bash
genalyzer fourier analyze \
    --npy-path /tmp/adc_capture.npy \
    --sample-rate 250e6 \
    --window blackman_harris \
    --ssb 12 \
    --plot
```

### Sweep in a pipeline

```bash
for f in 10e6 30e6 60e6 90e6; do
    TONE=$(genalyzer generators test-tone --num-points 8192 --sample-rate 250e6 \
           --tone-freq "$f" --amplitude 0.9 --compact | jq -r .output_path)
    genalyzer fourier analyze --npy-path "$TONE" --sample-rate 250e6 \
        --window blackman_harris --ssb 3 --compact \
        | jq "{tone_freq: $f, sfdr, snr}"
done
```

## Exit codes

| Situation | Exit |
|---|---|
| Tool returns a result dict | `0` |
| Tool returns `{"error": "..."}` | `0` (stdout JSON carries the error for the agent) |
| Unhandled exception inside the tool | `1` (stdout JSON with an `error` key) |
| Click detects bad arguments (missing flag, wrong type) | `2` |
