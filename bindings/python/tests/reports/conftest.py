import pytest
from genalyzer.pytest.plugin import _plot_manager


@pytest.fixture
def gn_plot_manager(request):
    """
    Pytest fixture that provides plotly plotting capabilities.

    Usage in tests:
        def test_example(gn_plot_manager):
            import plotly.graph_objects as go
            fig = go.Figure(data=go.Bar(x=['A', 'B'], y=[1, 2]))
            gn_plot_manager.add_plot(fig, name="My Bar Chart")
    """
    test_id = request.node.nodeid
    _plot_manager.set_current_test(test_id)
    return _plot_manager