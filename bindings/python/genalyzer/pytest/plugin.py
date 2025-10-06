# Copyright (C) 2025 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later
"""
Pytest plugin for generating HTML reports with embedded plots.

This is based on pytest-reporter-html1 but heavily modified to support plotly
plots and to work with pytest-reporter.
"""

import hashlib
import itertools
import json
import mimetypes
import re
import shutil
import uuid
from base64 import b64encode, b64decode
from datetime import datetime, timedelta
from inspect import cleandoc
from pathlib import Path
import warnings
import logging


# logger = logging.getLogger(__name__)

# htmlmin2 removed as it's deprecated
import pytest
from ansi2html import Ansi2HTMLConverter
from ansi2html.style import get_styles
from docutils.core import publish_parts
from jinja2 import (
    Environment,
    FileSystemLoader,
    TemplateNotFound,
    ChainableUndefined,
    select_autoescape,
)
from markupsafe import Markup

from . import __version__

from .plots import gn_plot_fft_single_tone, add_plot, add_plot_img_file, add_plot_html

TEMPLATE_PATH = Path(__file__).parent / "templates"


class PlotManager:
    """Manages plotly plot data for tests."""

    def __init__(self):
        self._plots = {}
        self._current_test_id = None

        # Add plots
        self.gn_plot_fft_single_tone = gn_plot_fft_single_tone.__get__(self)
        self.add_plot = add_plot.__get__(self)
        self.add_plot_img_file = add_plot_img_file.__get__(self)
        self.add_plot_html = add_plot_html.__get__(self)

    def set_current_test(self, test_id):
        """Set the current test ID for plot association."""
        self._current_test_id = test_id
        if test_id not in self._plots:
            self._plots[test_id] = []

    def get_plots_for_test(self, test_id):
        """Get all plots for a specific test."""
        return self._plots.get(test_id, [])

    def get_all_plots(self):
        """Get all plots data."""
        return self._plots

    def update_images_to_asset_folder(self, asset_dir):
        """Copy image files to the specified asset directory."""
        asset_dir = Path(asset_dir)
        asset_dir = asset_dir / "plots"
        asset_dir.mkdir(parents=True, exist_ok=True)
        for test_id, plots in self._plots.items():
            for plot in plots:
                # logger.info(f"Processing plot {plot['id']} of type {plot['type']}")
                if plot["type"] == "image":
                    src_path = Path(plot["data"])
                    if src_path.is_file():
                        dest_path = asset_dir / src_path.name
                        shutil.move(src_path, dest_path)
                        # logger.info(f"Copied image {src_path} to {dest_path}")
                        plot["data"] = str(dest_path)  # Update path to new location
                    else:
                        warnings.warn(
                            f"Image file {src_path} does not exist, cannot copy."
                        )


# Global plot manager instance
_plot_manager = PlotManager()


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


# category/style: background-color, color
COLORS = {
    "passed": ("#43A047", "#FFFFFF"),
    "failed": ("#F44336", "#FFFFFF"),
    "error": ("#B71C1C", "#FFFFFF"),
    "xfailed": ("#EF9A9A", "#222222"),
    "xpassed": ("#A5D6A7", "#222222"),
    "skipped": ("#9E9E9E", "#FFFFFF"),
    "notrun": ("#9E9E9E", "#FFFFFF"),
    "rerun": ("#FBC02D", "#222222"),
    "warning": ("#FBC02D", "#222222"),
    "green": ("#43A047", "#FFFFFF"),
    "red": ("#E53935", "#FFFFFF"),
    "yellow": ("#FBC02D", "#222222"),
}


def pytest_addoption(parser):
    group = parser.getgroup("report generation")
    group.addoption(
        "--split-report",
        action="store_true",
        help="store CSS and image files separately from the HTML.",
    )
    group.addoption(
        "--reports-dir",
        action="store",
        default="test_reports",
        help="directory to store test reports and artifacts (default: test_reports).",
    )


def pytest_configure(config):
    # Only register the template plugin if pytest-reporter is available
    try:
        import pytest_reporter

        # Override template
        config.option.template = ["genalyzer/index.html"]
        config.pluginmanager.register(TemplatePlugin(config))
        pass  # TemplatePlugin registered successfully
    except ImportError:
        raise ImportError(
            "pytest-reporter is not installed. Please install it to use the reporting features."
        )
        pass  # pytest-reporter not available, skipping TemplatePlugin registration


# This function is called by pytest to register fixtures
def pytest_runtest_setup(item):
    """Called before each test item is run."""
    # Set the current test for plot management
    test_id = item.nodeid
    _plot_manager.set_current_test(test_id)


def css_minify(s):
    s = re.sub(r"\s+", " ", s)
    s = re.sub(r"/\*.*?\*/", "", s)
    return s


class TemplatePlugin:

    def __init__(self, config):
        self.self_contained = not config.getoption("--split-report")
        self.reports_dir = config.getoption("--reports-dir")
        self._css = None
        self._assets = []
        self._dirs = []

    def pytest_reporter_loader(self, dirs, config):
        self._dirs = dirs + [str(TEMPLATE_PATH)]
        conv = Ansi2HTMLConverter(escaped=False)
        self.env = env = Environment(
            loader=FileSystemLoader(self._dirs),
            autoescape=select_autoescape(["html", "htm", "xml"]),
            undefined=ChainableUndefined,
        )
        env.globals["get_ansi_styles"] = get_styles
        env.globals["self_contained"] = self.self_contained
        env.globals["__version__"] = __version__
        env.filters["css"] = self._cssfilter
        env.filters["asset"] = self._assetfilter
        env.filters["repr"] = repr
        env.filters["chain"] = itertools.chain.from_iterable
        env.filters["strftime"] = lambda ts, fmt: datetime.fromtimestamp(ts).strftime(
            fmt
        )
        env.filters["timedelta"] = lambda ts: timedelta(seconds=ts)
        env.filters["ansi"] = lambda s: conv.convert(s, full=False)
        env.filters["cleandoc"] = cleandoc
        env.filters["rst"] = lambda s: publish_parts(source=s, writer_name="html5")[
            "body"
        ]
        env.filters["css_minify"] = css_minify
        return env

    def pytest_reporter_context(self, context, config):
        context.setdefault("colors", COLORS)
        context.setdefault("time_format", "%Y-%m-%d %H:%M:%S")
        metadata = context.setdefault("metadata", {})

        if config.pluginmanager.getplugin("metadata"):
            from pytest_metadata.plugin import metadata_key

            metadata.update(config.stash[metadata_key])

        # Add plots data to context
        _plot_manager.update_images_to_asset_folder(self.reports_dir)
        plots_data = _plot_manager.get_all_plots()
        context["plots_data"] = plots_data

    def _cssfilter(self, css):
        if self.self_contained:
            return Markup("<style>") + css + Markup("</style>")
        else:
            self._css = css
            return Markup('<link rel="stylesheet" type="text/css" href="style.css">')

    def _assetfilter(self, src, extension=None, inline=None):
        path = None
        b64_content = None
        raw_content = None
        if inline is None:
            inline = self.self_contained

        if isinstance(src, bytes):
            raw_content = src
        elif len(src) > 255:
            # Probably not a path
            b64_content = src
        else:
            try:
                for parent in [".", *self._dirs]:
                    maybe_file = Path(parent) / src
                    if maybe_file.is_file():
                        path = maybe_file
                        break
                else:
                    b64_content = src
            except ValueError:
                b64_content = src

        if not path and not b64_content and not raw_content:
            warnings.warn("Could not find file")
            path = src

        if inline:
            if path:
                fname = str(path)
            elif extension:
                fname = "temp." + extension
            mimetype, _ = mimetypes.guess_type(fname)
            if not mimetype:
                mimetype = "application/octet-stream"
            if path:
                raw_content = path.read_bytes()
            if raw_content:
                b64_content = b64encode(raw_content).decode("utf-8")
            return "data:" + mimetype + ";base64," + b64_content
        else:
            m = hashlib.sha1()
            if path:
                with path.open("rb") as fp:
                    while True:
                        data = fp.read(16384)
                        if not data:
                            break
                        m.update(data)
                content = path
            if b64_content:
                raw_content = b64decode(b64_content.encode("utf-8"))
            if raw_content:
                m.update(raw_content)
                content = raw_content
            if extension:
                suffix = "." + extension
            else:
                suffix = path.suffix
            fname = m.hexdigest() + suffix
            self._assets.append((fname, content))
            return "assets/" + fname

    def pytest_reporter_render(self, template_name, dirs, context):
        # Override
        try:
            template = self.env.get_template(template_name)
        except TemplateNotFound:
            return
        html = template.render(context)
        # HTML minification removed (htmlmin2 is deprecated)
        return html

    def pytest_reporter_finish(self, path, context, config):
        # Use custom reports directory or fall back to path.parent
        reports_base = Path(self.reports_dir) if self.reports_dir else path.parent
        # logger.info("Saving report to:", reports_base)
        reports_base.mkdir(parents=True, exist_ok=True)

        assets = reports_base / "assets"
        if not self.self_contained:
            assets.mkdir(parents=True, exist_ok=True)
        if self._css:
            style_css = reports_base / "style.css"
            style_css.write_text(self._css)
        for fname, content in self._assets:
            if isinstance(content, bytes):
                with open(assets / fname, "wb") as fp:
                    fp.write(content)
            else:
                shutil.copy(content, assets / fname)

        # Save plot data files
        plots_data = _plot_manager.get_all_plots()
        if plots_data:
            if not self.self_contained:
                plots_dir = reports_base / "plots"
                # logger.info("Saving plots to:", plots_dir)
                plots_dir.mkdir(parents=True, exist_ok=True)
                for test_id, plots in plots_data.items():
                    for plot in plots:
                        plot_file = plots_dir / f"{plot['id']}.json"
                        plot_file.write_text(plot["data"])
            # For self-contained reports, plot data is embedded in the HTML
