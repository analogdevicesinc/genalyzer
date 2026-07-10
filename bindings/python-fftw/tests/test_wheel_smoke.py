from importlib import metadata, resources
from pathlib import Path

import numpy as np

import genalyzer as gn
import genalyzer.pygenalyzer as pygn


def test_runtime_metadata_and_files():
    runtime = metadata.distribution("genalyzer-fftw")
    api = metadata.distribution("genalyzer")
    assert runtime.metadata["License-Expression"] == "GPL-2.0-or-later"
    assert runtime.metadata["Name"] == "genalyzer-fftw"
    assert api.metadata["License-Expression"] == "LicenseRef-ADIBSD OR GPL-2.0-or-later"

    runtime_files = {str(path).replace("\\", "/") for path in runtime.files or ()}
    api_files = {str(path).replace("\\", "/") for path in api.files or ()}
    assert not {p for p in runtime_files if p.startswith("genalyzer/")}
    assert not (runtime_files & api_files)
    for name in (
        "LICENSE",
        "FFTW_COPYING",
        "THIRD_PARTY_LICENSES.md",
        "FFTW_SOURCE.md",
    ):
        assert any(path.endswith("/" + name) for path in runtime_files)


def test_runtime_loads_and_executes_fft():
    runtime_dir = Path(str(resources.files("genalyzer_fftw"))).resolve()
    library_path = Path(pygn._libpath).resolve()
    assert library_path.is_file()
    assert library_path.is_relative_to(runtime_dir)

    installed = [str(path).lower() for path in metadata.files("genalyzer-fftw") or ()]
    assert any(
        "fftw3" in path and (".so" in path or path.endswith((".dll", ".dylib")))
        for path in installed
    )

    samples = np.ones(8, dtype=np.complex128)
    result = gn.fft(samples, 1, 8, gn.Window.NO_WINDOW)
    np.testing.assert_allclose(result[0], 1)
    np.testing.assert_allclose(result[1:], 0, atol=1e-12)
