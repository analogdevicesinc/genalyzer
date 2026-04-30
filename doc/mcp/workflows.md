# Workflows

Three runnable patterns for the three target use cases. Each example here is exercised verbatim by `tests/test_mcp_workflows.py`.

## Analyze hardware captures

Capture IQ samples from hardware (e.g. via [pyadi-iio](https://github.com/analogdevicesinc/pyadi-iio)), save them as `.npy`, and run the one-shot analyzer.

```python
import numpy as np
from genalyzer.mcp.fourier import analyze_spectrum

# iq_samples: complex128 ndarray captured at 250 MSPS
np.save("/tmp/adc_capture.npy", iq_samples)

result = analyze_spectrum(
    npy_path="/tmp/adc_capture.npy",
    sample_rate=250e6,
    window="blackman_harris",
    ssb=12,      # absorb NCO imprecision and finite-resolution spread
    plot=True,   # set False in batch pipelines
)

print(result["sfdr"], result["snr"], result["enob"])
```

**Parameter notes for real hardware data:**

- `ssb=12` groups the NCO spillover into the fundamental bin. Use small values (e.g. `ssb=3`) only for ideal synthetic data.
- Capture data is normalized internally; a small DC offset does not harm SFDR.
- For real (non-IQ) ADCs, save a real-valued array — `analyze_spectrum` picks `rfft` automatically.

## Simulate-and-verify converter designs

Describe a converter, synthesize a clean tone, apply the quantizer, and verify the measured metrics against the spec.

```python
from genalyzer.mcp.fourier import analyze_spectrum
from genalyzer.mcp.generators import generate_real_tone
from genalyzer.mcp.quantize import quantize

tone = generate_real_tone(
    num_points=8192,
    sample_rate=250e6,
    tone_freq=30e6,
    amplitude=0.9,
    output_path="/tmp/tone.npy",
)

q = quantize(
    npy_path=tone["output_path"],
    bits=12,
    fullscale=2.0,          # fullscale spans [-1, 1] when amplitude = 0.9
    output_path="/tmp/tone.q.npy",
)

result = analyze_spectrum(
    npy_path=q["output_path"],
    sample_rate=250e6,
    window="blackman_harris",
    ssb=3,                 # synthetic, coherent — tight grouping
)

# An ideal 12-bit quantizer: ENOB ~= 12, SNR ~= 74 dB
assert 11.5 < result["enob"] < 12.5
```

Add `analyze_dnl`, `analyze_inl`, or `analyze_histogram` calls on `q["output_path"]` to close the loop on code-density metrics too.

## Automated characterization pipelines

Sweep a parameter (frequency, NFFT, bits, …) from an MCP-driven script, gathering deterministic JSON without plot overhead.

```python
from genalyzer.mcp.fourier import analyze_spectrum
from genalyzer.mcp.generators import generate_test_tone

freqs = [10e6, 30e6, 60e6, 90e6]
sample_rate = 250e6
results = []

for f in freqs:
    gen = generate_test_tone(
        num_points=8192,
        sample_rate=sample_rate,
        tone_freq=f,
        amplitude=0.9,
    )
    res = analyze_spectrum(
        npy_path=gen["output_path"],
        sample_rate=sample_rate,
        window="blackman_harris",
        ssb=3,
        plot=False,   # no matplotlib work in a sweep
    )
    results.append({"tone_freq": f, "sfdr": res["sfdr"], "snr": res["snr"]})

for r in results:
    print(r)
```

From an interactive assistant, this same pattern is just "for each of these frequencies, generate a tone and run analyze_spectrum; return the SFDR and SNR".
