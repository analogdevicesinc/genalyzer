# Copyright (C) 2025 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""Special plot implementations for pytest-genalyzer."""

import numpy as np

import uuid
import warnings
from pathlib import Path

import genalyzer as gn


def gn_plot_fft_single_tone(
    self,
    data,
    full_scale,
    sample_rate,
    tone_frequency,
    ssb_rest=3,
    ssb_fund=4,
    nfft=None,
    navg=1,
    plot_height_px=1000,
    title="SFDR Plot",
):
    """Plot SFDR (Spurious-Free Dynamic Range) using plotly.

    Args:
        ``data`` (dict): Dictionary containing frequency and magnitude data.

        ``full_scale`` (float): Full scale value for normalization.

        ``sample_rate`` (float): Sample rate of the signal.

        ``tone_frequency`` (float): Frequency of the tone to analyze.

        ``ssb_rest`` (int, optional): Sideband setting for rest of the spectrum. Defaults to 3.

        ``ssb_fund`` (int, optional): Sideband setting for fundamental tone. Defaults to 4.

        ``nfft`` (int, optional): Number of FFT points. If None, it will be set to len(data)//2. Defaults to None.

        ``navg`` (int, optional): Number of averages. Defaults to 1.

        ``plot_height_px`` (int, optional): Height of the plot in pixels. Defaults to 1000.
        
        ``title`` (str): Title of the plot.
    """

    # Genalyzer side

    # Defaults
    fshift = 0.0
    fdata = sample_rate / 1
    fs = sample_rate
    freq = tone_frequency
    # navg = 1
    if nfft is None:
        nfft = len(data) // 2

    assert nfft * navg * 2 == len(data), "nfft * navg * 2 must equal length of data"

    code_fmt = gn.CodeFormat.TWOS_COMPLEMENT
    axis_fmt = gn.FreqAxisFormat.FREQ
    axis_type = gn.FreqAxisType.DC_CENTER
    window = gn.Window.NO_WINDOW

    if gn.Window.NO_WINDOW == window:
        freq = gn.coherent(nfft, fdata, freq)
        fbin = fdata / nfft
        fshift = round(fshift / fbin) * fbin
        ssb_fund = 0
        ssb_rest = 0

    # Convert to bits
    qres = int(np.log2(full_scale))

    key = "fa"
    gn.mgr_remove(key)
    gn.fa_create(key)
    gn.fa_analysis_band(key, "fdata*0.0", "fdata*1.0")
    gn.fa_fixed_tone(key, "A", gn.FaCompTag.SIGNAL, freq, ssb_fund)
    # gn.fa_max_tone(key, 'A', gn.FaCompTag.SIGNAL, ssb_fund)
    gn.fa_conv_offset(key, 0.0 != fshift)
    gn.fa_hd(key, 3)
    gn.fa_ssb(key, gn.FaSsb.DEFAULT, ssb_rest)
    gn.fa_ssb(key, gn.FaSsb.DC, -1)
    gn.fa_ssb(key, gn.FaSsb.SIGNAL, -1)
    gn.fa_ssb(key, gn.FaSsb.WO, -1)
    gn.fa_fdata(key, fdata)
    gn.fa_fsample(key, fs)
    gn.fa_fshift(key, fshift)
    # print(gn.fa_preview(key, True))

    fft_cplx = gn.fft(data, qres, navg, nfft, window, code_fmt)

    results = gn.fft_analysis(key, fft_cplx, nfft, axis_type)

    freq_axis = gn.freq_axis(nfft, axis_type, fdata, axis_fmt)
    fft_db = gn.db(fft_cplx)
    if gn.FreqAxisType.DC_CENTER == axis_type:
        fft_db = gn.fftshift(fft_db)
    annots = gn.fa_annotations(results, axis_type, axis_fmt)

    # # Matplotlib side
    # import matplotlib.pyplot as pl
    # from matplotlib.patches import Rectangle as MPRect

    # fig = pl.figure(1)
    # fig.clf()
    # pl.plot(freq_axis, fft_db)
    # pl.grid(True)
    # pl.xlim(freq_axis[0], freq_axis[-1])
    # pl.ylim(-140.0, 20.0)
    # for x, y, label in annots["labels"]:
    #     pl.annotate(label, xy=(x, y), ha="center", va="bottom")
    # for line in annots["lines"]:
    #     pl.axline((line[0], line[1]), (line[2], line[3]), c="pink")
    # for box in annots["ab_boxes"]:
    #     fig.axes[0].add_patch(
    #         MPRect(
    #             (box[0], box[1]),
    #             box[2],
    #             box[3],
    #             ec="lightgray",
    #             fc="gainsboro",
    #             fill=True,
    #             hatch="x",
    #         )
    #     )
    # for box in annots["tone_boxes"]:
    #     fig.axes[0].add_patch(
    #         MPRect(
    #             (box[0], box[1]),
    #             box[2],
    #             box[3],
    #             ec="pink",
    #             fc="pink",
    #             fill=True,
    #             hatch="x",
    #         )
    #     )
    # pl.show()

    # Implement same plot using plotly
    import plotly.graph_objects as go
    from plotly.subplots import make_subplots

    # fig = go.Figure()
    fig = make_subplots(
        rows=2,
        cols=1,
        vertical_spacing=0.15,
        specs=[[{"type": "scatter"}], [{"type": "table"}]],
    )

    fig.add_trace(go.Scatter(x=freq_axis, y=fft_db, mode="lines"), row=1, col=1)
    fig.update_layout(
        title="FFT Analysis",
        xaxis_title="Frequency (Hz)",
        yaxis_title="Magnitude (dBFS)",
        height=plot_height_px,
    )
    for x, y, label in annots["labels"]:
        fig.add_annotation(x=x, y=y, text=label, showarrow=True)
    # for line in annots["lines"]:
    #     fig.add_shape(type="line", x0=line[0], y0=line[1], x1=line[2], y1=line[3], line=dict(color="pink"))
    # for box in annots["ab_boxes"]:
    #     fig.add_shape(type="rect", x0=box[0], y0=box[1], x1=box[2], y1=box[3], line=dict(color="lightgray"), fillcolor="gainsboro", opacity=0.5)
    # for box in annots["tone_boxes"]:
    #     fig.add_shape(type="rect", x0=box[0], y0=box[1], x1=box[2], y1=box[3], line=dict(color="pink"), fillcolor="pink", opacity=0.5)

    # Add table with results
    table_header = dict(values=["Metric", "Value"], align="left")
    table_cells = dict(
        values=[list(results.keys()), [f"{v:.6f}" for v in results.values()]],
        fill_color="lavender",
        align="left",
    )
    fig.add_trace(go.Table(header=table_header, cells=table_cells), row=2, col=1)

    self.add_plot(fig, name="FFT Analysis (plotly)")


def add_plot(self, fig, name=None):
    """Add a plotly figure to the current test report.
    
    Args:
        ``fig`` (``plotly.graph_objects.Figure``): The plotly figure to add.

        ``name`` (``str, optional``): Name of the plot. If None, a default name will be assigned.
    """
    if self._current_test_id is None:
        warnings.warn("No current test set for plot. Use gn_plot_manager to create plots.")
        return None

    plot_id = str(uuid.uuid4())
    plot_data = {
        "id": plot_id,
        "type": "plotly",
        "name": name or f"Plot {len(self._plots[self._current_test_id]) + 1}",
        "data": fig.to_json(),
    }
    self._plots[self._current_test_id].append(plot_data)
    return plot_id


def add_plot_img_file(self, img_path, name=None):
    """Add an image file as a plot to the current test report.
    
    Args:
        ``img_path`` (``str``): Path to the image file.

        ``name`` (``str``, optional): Name of the plot. If None, a default name will be assigned.
    """
    if self._current_test_id is None:
        warnings.warn("No current test set for plot. Use gn_plot_manager to create plots.")
        return None

    if not Path(img_path).is_file():
        warnings.warn(f"Image file {img_path} does not exist.")
        return None

    plot_id = str(uuid.uuid4())
    plot_data = {
        "id": plot_id,
        "type": "image",
        "name": name or f"Image {len(self._plots[self._current_test_id]) + 1}",
        "data": img_path,
    }
    self._plots[self._current_test_id].append(plot_data)
    return plot_id


def add_plot_html(self, html_str, name=None):
    """Add raw HTML content as a plot to the current test report.

    Args:
        ``html_str`` (``str``): HTML content as a string.

        ``name`` (``str``, optional): Name of the plot. If None, a default name will be assigned.
    """
    if self._current_test_id is None:
        warnings.warn("No current test set for plot. Use gn_plot_manager to create plots.")
        return None

    plot_id = str(uuid.uuid4())
    plot_data = {
        "id": plot_id,
        "type": "html",
        "name": name or f"HTML {len(self._plots[self._current_test_id]) + 1}",
        "data": html_str,
    }
    self._plots[self._current_test_id].append(plot_data)
    return plot_id
