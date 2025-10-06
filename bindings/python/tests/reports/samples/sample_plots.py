# SPDX-License-Identifier: Apache-2.0
#
# Ported from https://github.com/christiansandberg/pytest-reporter-html1
import pytest
import plotly.graph_objects as go

try:
    import plotly.express as px
    import numpy as np
    import pandas as pd
    HAS_NUMPY_PANDAS = True
except ImportError:
    HAS_NUMPY_PANDAS = False
    # Create mock objects for tests that don't require numpy/pandas
    np = None
    pd = None
    px = None


def test_simple_bar_chart(gn_plot_manager):
    """Test that creates a simple bar chart."""
    fig = go.Figure(data=go.Bar(x=['A', 'B', 'C'], y=[1, 3, 2]))
    fig.update_layout(title="Simple Bar Chart")
    gn_plot_manager.add_plot(fig, name="Bar Chart Example")
    assert True


def test_basic_scatter_plot(gn_plot_manager):
    """Test with basic scatter plot using simple lists."""
    x_values = [1, 2, 3, 4, 5]
    y_values = [2, 5, 3, 8, 7]

    fig = go.Figure(data=go.Scatter(
        x=x_values,
        y=y_values,
        mode='markers',
        marker=dict(size=10, color='blue')  # Make markers visible
    ))
    fig.update_layout(
        title="Basic Scatter Plot",
        xaxis_title="X Values",
        yaxis_title="Y Values"
    )
    gn_plot_manager.add_plot(fig, name="Simple Scatter")

    assert len(x_values) == 5


def test_medium_scatter_plot(gn_plot_manager):
    """Test scatter plot with a medium-sized dataset to verify visibility."""
    import random

    n_points = 1000
    x_values = list(range(n_points))
    y_values = [random.random() * 10 + i * 0.01 for i in x_values]  # Trend with noise

    fig = go.Figure()
    fig.add_trace(go.Scatter(
        x=x_values,
        y=y_values,
        mode='markers',
        marker=dict(
            size=4,
            color=y_values,  # Color by y-value
            colorscale='Viridis',
            opacity=0.7
        ),
        name=f'{n_points} Points'
    ))

    fig.update_layout(
        title=f"Medium Dataset: {n_points} Points",
        xaxis_title="Index",
        yaxis_title="Value"
    )

    gn_plot_manager.add_plot(fig, name="Medium Scatter")

    assert len(x_values) == n_points


def test_pie_chart(gn_plot_manager):
    """Test with a pie chart."""
    labels = ['Apple', 'Banana', 'Cherry', 'Date']
    values = [30, 25, 20, 25]

    fig = go.Figure(data=go.Pie(labels=labels, values=values))
    fig.update_layout(title="Fruit Distribution")
    gn_plot_manager.add_plot(fig, name="Pie Chart")

    assert sum(values) == 100


@pytest.mark.skipif(not HAS_NUMPY_PANDAS, reason="requires numpy and pandas")
def test_line_chart_with_multiple_traces(gn_plot_manager):
    """Test with multiple plotly charts."""
    # First plot: Line chart
    x = np.linspace(0, 10, 100)
    y1 = np.sin(x)
    y2 = np.cos(x)

    fig1 = go.Figure()
    fig1.add_trace(go.Scatter(x=x, y=y1, name='sin(x)'))
    fig1.add_trace(go.Scatter(x=x, y=y2, name='cos(x)'))
    fig1.update_layout(title="Trigonometric Functions")
    gn_plot_manager.add_plot(fig1, name="Trigonometric Functions")

    # Second plot: Histogram
    data = np.random.normal(0, 1, 1000)
    fig2 = go.Figure(data=go.Histogram(x=data, nbinsx=30))
    fig2.update_layout(title="Random Normal Distribution")
    gn_plot_manager.add_plot(fig2, name="Histogram")

    assert len(x) == 100


@pytest.mark.skipif(not HAS_NUMPY_PANDAS, reason="requires numpy and pandas")
def test_scatter_plot_with_pandas(gn_plot_manager):
    """Test using plotly express with pandas data."""
    # Create sample data
    df = pd.DataFrame({
        'x': np.random.randn(100),
        'y': np.random.randn(100),
        'category': np.random.choice(['A', 'B', 'C'], 100)
    })

    fig = px.scatter(df, x='x', y='y', color='category', title="Scatter Plot by Category")
    gn_plot_manager.add_plot(fig, name="Scatter Plot")

    assert len(df) == 100


def test_no_plots():
    """Test that works without any plots."""
    assert True


def test_multiple_plots_per_test(gn_plot_manager):
    """Test that creates multiple plots in one test."""
    # First plot: Simple bar chart
    fig1 = go.Figure(data=go.Bar(x=['Red', 'Blue', 'Green'], y=[10, 15, 8]))
    fig1.update_layout(title="Color Preferences")
    gn_plot_manager.add_plot(fig1, name="Bar Chart")

    # Second plot: Line chart with basic data
    fig2 = go.Figure(data=go.Scatter(x=[1, 2, 3, 4], y=[10, 11, 12, 13], mode='lines+markers'))
    fig2.update_layout(title="Simple Trend")
    gn_plot_manager.add_plot(fig2, name="Line Chart")

    assert True


def test_failing_test_with_plot(gn_plot_manager):
    """Test that fails but includes a plot for debugging."""
    # Create a diagnostic plot
    x = [1, 2, 3, 4, 5]
    expected = [2, 4, 6, 8, 10]
    actual = [2, 4, 7, 8, 9]  # Different from expected

    fig = go.Figure()
    fig.add_trace(go.Scatter(x=x, y=expected, name='Expected', mode='lines+markers'))
    fig.add_trace(go.Scatter(x=x, y=actual, name='Actual', mode='lines+markers'))
    fig.update_layout(title="Expected vs Actual Values")
    gn_plot_manager.add_plot(fig, name="Debug Plot")

    # This assertion will fail
    assert actual == expected, "Values don't match - see plot for visualization"


@pytest.mark.skipif(not HAS_NUMPY_PANDAS, reason="requires numpy and pandas")
@pytest.mark.parametrize("n", [10, 20, 30])
def test_parametrized_with_plots(gn_plot_manager, n):
    """Test that creates different plots based on parameter."""
    x = np.arange(n)
    y = x ** 2

    fig = go.Figure(data=go.Scatter(x=x, y=y, mode='lines+markers'))
    fig.update_layout(title=f"Quadratic Function (n={n})")
    gn_plot_manager.add_plot(fig, name=f"Quadratic n={n}")

    assert len(x) == n


@pytest.mark.skipif(not HAS_NUMPY_PANDAS, reason="requires numpy and pandas")
@pytest.mark.slow
def test_large_dataset_stress_test(gn_plot_manager):
    """Stress test with 2^20 (1,048,576) data points to test performance."""
    n_samples = 2**20  # 1,048,576 points

    # Generate large dataset
    x = np.arange(n_samples, dtype=np.float32)
    # Create a function with some interesting patterns
    y = np.sin(x / 10000) + 0.1 * np.random.randn(n_samples).astype(np.float32)

    # Create scatter plot with full dataset
    fig = go.Figure()
    fig.add_trace(go.Scattergl(
        x=x,
        y=y,
        mode='markers',
        marker=dict(size=2, opacity=0.6),
        name=f'Full Dataset ({n_samples:,} points)'
    ))

    fig.update_layout(
        title=f"Stress Test: {n_samples:,} Data Points (Full Dataset)",
        xaxis_title="Sample Index",
        yaxis_title="Value",
        showlegend=True
    )

    gn_plot_manager.add_plot(fig, name=f"Stress Test {n_samples//1000}K Points")

    # Create a line plot with full dataset
    fig_line = go.Figure()
    fig_line.add_trace(go.Scatter(
        x=x,
        y=y,
        mode='lines',
        line=dict(width=0.5),
        name=f'Full Signal ({n_samples:,} points)'
    ))

    fig_line.update_layout(
        title=f"Signal Pattern from {n_samples:,} Data Points (Full Dataset)",
        xaxis_title="Sample Index",
        yaxis_title="Value",
        showlegend=True
    )

    gn_plot_manager.add_plot(fig_line, name="Signal Pattern")

    # Histogram of full dataset (histograms handle large datasets well)
    fig_hist = go.Figure()
    fig_hist.add_trace(go.Histogram(
        x=y,  # Use full dataset for accurate distribution
        nbinsx=100,
        name="Value Distribution"
    ))
    fig_hist.update_layout(
        title=f"Distribution of {n_samples:,} Data Points",
        xaxis_title="Value",
        yaxis_title="Frequency"
    )

    gn_plot_manager.add_plot(fig_hist, name="Distribution Histogram")

    # Verify we have the full dataset
    assert len(x) == n_samples
    assert len(y) == n_samples