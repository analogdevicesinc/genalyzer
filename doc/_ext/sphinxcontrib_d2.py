"""Sphinx directive that renders d2 diagrams via pyd2lang-native.

Registers ``.. d2::`` and emits dual-theme (light + dark) SVGs wrapped in
``only-light`` / ``only-dark`` divs provided by the adi_doctools cosmic theme.
"""

from __future__ import annotations

import hashlib
import os
import pathlib
import re
import shutil
import tempfile

import d2
from docutils import nodes
from sphinx.application import Sphinx
from sphinx.errors import ExtensionError
from sphinx.util.docutils import SphinxDirective

_CSS_FILENAME = "sphinxcontrib_d2.css"
_CSS_SOURCE = pathlib.Path(__file__).with_name(_CSS_FILENAME)


_XML_PROLOG_RE = re.compile(r"^\s*<\?xml[^?]*\?>\s*", re.DOTALL)
_ROOT_SVG_OPEN_RE = re.compile(r"<svg\b([^>]*)>", re.IGNORECASE)
_ID_ATTR_RE = re.compile(r'\bid="([^"]+)"')


def _rewrite_root_svg(match: "re.Match[str]") -> str:
    """Strip width/height/fill from the root <svg> and inject responsive style."""
    attrs = match.group(1)
    attrs = re.sub(r'\s+(width|height|fill)="[^"]*"', "", attrs)

    responsive = "max-width:100%;height:auto"
    style_match = re.search(r'style="([^"]*)"', attrs)
    if style_match:
        existing = style_match.group(1).rstrip(";")
        new_style = f'style="{existing};{responsive}"' if existing else f'style="{responsive}"'
        attrs = attrs[: style_match.start()] + new_style + attrs[style_match.end() :]
    else:
        attrs = attrs.rstrip() + f' style="{responsive}"'
    return f"<svg{attrs}>"


def _rewrite_ids(svg: str, nonce: str) -> str:
    """Prefix every id/url(#…)/href="#…" in the SVG with ``nonce-``.

    Replacements are supplied as lambdas so ``re.sub`` never interprets
    backslash escapes in ``nonce-{id}`` (e.g., an id like ``\\1x`` would
    otherwise be seen as a backreference).
    """
    ids = set(_ID_ATTR_RE.findall(svg))
    for original in ids:
        replacement = f"{nonce}-{original}"
        escaped = re.escape(original)
        svg = re.sub(
            rf'\bid="{escaped}"',
            lambda m, r=replacement: f'id="{r}"',
            svg,
        )
        svg = re.sub(
            rf"url\(#{escaped}\)",
            lambda m, r=replacement: f"url(#{r})",
            svg,
        )
        svg = re.sub(
            rf'(xlink:)?href="#{escaped}"',
            lambda m, r=replacement: f'{m.group(1) or ""}href="#{r}"',
            svg,
        )
    return svg


def _post_process(svg: str, nonce: str) -> str:
    """Prepare a raw d2 SVG for inline HTML embedding."""
    svg = _XML_PROLOG_RE.sub("", svg)
    svg = _ROOT_SVG_OPEN_RE.sub(_rewrite_root_svg, svg, count=1)
    svg = _rewrite_ids(svg, nonce)
    return svg


def _compile_raw(source: str, theme: str) -> str:
    """Compile d2 source with the SW library at a given theme.

    Raises RuntimeError if the compiler returns an error string or None.
    """
    svg = d2.compile(source, library="sw", theme=theme)
    if svg is None:
        raise RuntimeError(f"d2.compile returned None for theme={theme!r}")
    if svg.startswith("Error"):
        raise RuntimeError(svg)
    return svg


def _compile_raw_cached(source: str, theme: str, cache_dir: pathlib.Path) -> str:
    """Return raw d2 SVG for (source, theme), using an on-disk cache.

    Writes are atomic (temp-file + ``os.replace``) so parallel Sphinx
    workers that race on the same cache entry never leave a partial file
    visible to readers.
    """
    cache_dir = pathlib.Path(cache_dir)
    cache_dir.mkdir(parents=True, exist_ok=True)

    key_src = f"{theme}\n{source}".encode("utf-8")
    digest = hashlib.sha256(key_src).hexdigest()
    cache_file = cache_dir / f"{digest}.svg"
    if cache_file.exists():
        return cache_file.read_text(encoding="utf-8")

    svg = _compile_raw(source, theme=theme)
    tmp_fd, tmp_path = tempfile.mkstemp(prefix=f".{digest}.", suffix=".svg.tmp", dir=cache_dir)
    try:
        with os.fdopen(tmp_fd, "w", encoding="utf-8") as fh:
            fh.write(svg)
        os.replace(tmp_path, cache_file)
    except Exception:
        # Best-effort cleanup if replace never ran.
        try:
            os.unlink(tmp_path)
        except OSError:
            pass
        raise
    return svg


def _render_diagram(source: str, cache_dir: pathlib.Path, diagram_index: int) -> str:
    """Compile source for both themes and return an HTML fragment with both SVGs."""
    pieces: list[str] = []
    for theme in ("light", "dark"):
        try:
            raw = _compile_raw_cached(source, theme=theme, cache_dir=cache_dir)
        except RuntimeError as exc:
            raise ExtensionError(f"d2 compile failed ({theme}): {exc}") from exc
        nonce = f"d2-{diagram_index}-{theme}"
        svg = _post_process(raw, nonce=nonce)
        pieces.append(f'<div class="d2-diagram only-{theme}">{svg}</div>')
    return "\n".join(pieces)


class D2Directive(SphinxDirective):
    """Render an inline d2 diagram via pyd2lang-native."""

    has_content = True
    required_arguments = 0
    optional_arguments = 0
    final_argument_whitespace = False

    def run(self) -> list[nodes.Node]:
        source = "\n".join(self.content)
        env = self.env
        counter = env.temp_data.setdefault("d2_diagram_counter", 0)
        env.temp_data["d2_diagram_counter"] = counter + 1

        cache_dir = pathlib.Path(env.app.outdir).parent / ".d2-cache"
        html = _render_diagram(source, cache_dir=cache_dir, diagram_index=counter)
        return [nodes.raw("", html, format="html")]


def _install_css(app: Sphinx) -> None:
    """Copy the extension's CSS into the HTML build's ``_static`` directory."""
    if app.builder.name != "html":
        return
    dst_dir = pathlib.Path(app.outdir) / "_static"
    dst_dir.mkdir(parents=True, exist_ok=True)
    shutil.copy(_CSS_SOURCE, dst_dir / _CSS_FILENAME)


def setup(app: Sphinx) -> dict:
    app.add_directive("d2", D2Directive)
    app.connect("builder-inited", _install_css)
    app.add_css_file(_CSS_FILENAME)
    return {
        "version": "0.1.0",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
