# doc/_ext/test_sphinxcontrib_d2.py
import pytest

pytest.importorskip("d2")


def test_compile_raw_returns_svg_for_light_and_dark():
    from sphinxcontrib_d2 import _compile_raw

    light = _compile_raw("a -> b", theme="light")
    dark = _compile_raw("a -> b", theme="dark")

    assert "<svg" in light
    assert "</svg>" in light
    assert "<svg" in dark
    assert "</svg>" in dark


def test_post_process_strips_xml_prolog_and_adds_responsive_style():
    from sphinxcontrib_d2 import _compile_raw, _post_process

    raw = _compile_raw("a -> b", theme="light")
    assert raw.startswith("<?xml"), "sanity: d2 emits an XML prolog"

    out = _post_process(raw, nonce="n1")

    assert not out.lstrip().startswith("<?xml")
    assert out.lstrip().startswith("<svg")
    assert "max-width:100%" in out[:300]
    assert "height:auto" in out[:300]


def _extract_ids(svg: str) -> set[str]:
    import re

    return set(re.findall(r'\bid="([^"]+)"', svg))


def test_post_process_isolates_ids_per_nonce():
    from sphinxcontrib_d2 import _compile_raw, _post_process

    raw = _compile_raw("a -> b", theme="light")
    first = _post_process(raw, nonce="n1")
    second = _post_process(raw, nonce="n2")

    first_ids = _extract_ids(first)
    second_ids = _extract_ids(second)

    assert first_ids, "post-processed SVG should have some id= attributes"
    assert first_ids.isdisjoint(second_ids), (
        f"expected disjoint id sets, got intersection={first_ids & second_ids}"
    )
    # Every id from the first run must start with its nonce prefix.
    assert all(i.startswith("n1-") for i in first_ids)
    assert all(i.startswith("n2-") for i in second_ids)

    import re as _re

    first_urls = set(_re.findall(r"url\(#([^)]+)\)", first))
    second_urls = set(_re.findall(r"url\(#([^)]+)\)", second))
    # Every url(#…) reference that points to an id we rewrote must carry the nonce.
    # (Some CSS url(#…) refs may point to non-id constructs; we only assert the nonced ones.)
    first_nonced_urls = {u for u in first_urls if u.startswith("n1-")}
    second_nonced_urls = {u for u in second_urls if u.startswith("n2-")}
    assert first_nonced_urls.isdisjoint(second_nonced_urls)

    first_hrefs = set(_re.findall(r'href="#([^"]+)"', first))
    second_hrefs = set(_re.findall(r'href="#([^"]+)"', second))
    first_nonced_hrefs = {h for h in first_hrefs if h.startswith("n1-")}
    second_nonced_hrefs = {h for h in second_hrefs if h.startswith("n2-")}
    assert first_nonced_hrefs.isdisjoint(second_nonced_hrefs)


def test_compile_raw_cached_uses_disk_cache(tmp_path, monkeypatch):
    from sphinxcontrib_d2 import _compile_raw_cached

    calls = {"n": 0}

    def fake_compile(code, *, library=None, theme="light"):
        calls["n"] += 1
        return f"<svg theme={theme} code={code}></svg>"

    import sphinxcontrib_d2 as ext
    monkeypatch.setattr(ext.d2, "compile", fake_compile)

    first = _compile_raw_cached("a -> b", theme="light", cache_dir=tmp_path)
    second = _compile_raw_cached("a -> b", theme="light", cache_dir=tmp_path)

    assert first == second
    assert calls["n"] == 1, "second call should hit the cache"
    # A different theme should miss the cache and invoke d2 again.
    _compile_raw_cached("a -> b", theme="dark", cache_dir=tmp_path)
    assert calls["n"] == 2


def test_render_diagram_produces_dual_themed_divs(tmp_path):
    from sphinxcontrib_d2 import _render_diagram

    html = _render_diagram("a -> b", cache_dir=tmp_path, diagram_index=0)

    assert html.count("<svg") >= 2, "expected both light and dark SVGs inline"
    assert 'class="d2-diagram only-light"' in html
    assert 'class="d2-diagram only-dark"' in html
    # Indices differ so nonces differ between the two themes.
    assert html.index("only-light") < html.index("only-dark")
