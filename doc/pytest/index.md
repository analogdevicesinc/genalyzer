# Pytest Integration

To help with testing and reporting, Genalyzer provides a pytest plugin that allows you to create and attach plots directly to your test reports. This is particularly useful for visualizing signal analysis results during automated testing.

## Installation

To use the pytest plugin, install the optional `pytest` dependency:

```bash
pip install genalyzer[pytest]
```

## Usage

To use the pytest plugin, you need to include the `gn_plot_manager` in your test functions. This fixture provides methods to create and attach plots to your test reports.

Here is an example of how to use the `gn_plot_manager` in a test function:

```python
import numpy as np

def test_signal_analysis(gn_plot_manager):
    # Generate a sample signal
    t = np.linspace(0, 1, 500)
    signal = np.sin(2 * np.pi * 50 * t) + 0.5 * np.random.randn(500)

    # Create a plot of the signal
    fig, ax = gn_plot_manager.create_figure()
    ax.plot(t, signal)
    ax.set_title("Sample Signal")
    ax.set_xlabel("Time [s]")
    ax.set_ylabel("Amplitude")

    # Attach the plot to the test report
    gn_plot_manager.attach_figure(fig, "Sample Signal Plot")
```

### Addition methods

The `gn_plot_manager` fixture provides the following methods:

```{eval-rst}

.. automodule:: genalyzer.pytest.plots
   :members:
   :undoc-members:
   :show-inheritance:

```