# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath('../bindings/c/include'))
sys.path.insert(0, os.path.abspath('../bindings/python'))
sys.path.insert(0, os.path.dirname(__file__))

# Generate API page for documentation
from gen_api_page import gen_pages
gen_pages()

# -- Project information -----------------------------------------------------

project = "Genalyzer"
copyright = "2024-2025, Analog Devices, Inc."
author = "Analog Devices, Inc."

# The full version, including alpha/beta/rc tags
release = "v0.1.2"


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ["breathe", "sphinx.ext.autodoc", "myst_parser", "sphinx_inline_tabs", "sphinx.ext.graphviz", "sphinxcontrib.mermaid",  "sphinx_togglebutton"]

# Fix use of autogenerated cross-reference warning
myst_heading_anchors = 2

# Add any paths that contain templates here, relative to this directory.
templates_path = ["_templates"]

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = "furo"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ["_static"]

# Breathe Configuration
breathe_projects = { "Genalyzer": "../doxygen/xml" }
breathe_default_project = "Genalyzer"
