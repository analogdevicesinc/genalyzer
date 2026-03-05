# Installation

## Windows

We provide an installer for Windows. You can download it from the [releases page](https://github.com/analogdevicesinc/genalyzer/releases). There are installers built for each main branch commit. These are downloadable from the [Actions](https://github.com/analogdevicesinc/genalyzer/actions/workflows/test-win.yml) of each commit.

## Linux/macOS

For now we do not provide binaries for Linux or macOS. You can build the library from source. Here is the process for Ubuntu/macOS. If you are using another distribution, you will need to adapt the commands. Please send a PR if you have the commands for another distribution.

### Core Dependencies

````{tab} Ubuntu

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libfftw3-dev git
```
````

````{tab} macOS

```bash
brew install cmake fftw
```
````


### Get the source code

```bash
git clone https://github.com/analogdevicesinc/genalyzer.git
cd genalyzer
```

### (Optional) Python Dependencies

````{tab} Ubuntu

```bash
sudo apt-get install python3-dev python3-pip
sudo pip3 install numpy
```
````

````{tab} macOS

```bash
pip3 install numpy
```
````


### (Optional) Documentation Dependencies

```bash
sudo pip3 install -r requirements_doc.txt
```

### Build

```bash
mkdir build
cd build
cmake ..
make
sudo make install
```

#### CMake Options

| Option | Description | Default |
| ------ | ----------- | ------- |
| `BUILD_DOC` | Build the documentation | `OFF` |
| `BUILD_TESTS_EXAMPLES` | Build the tests and examples | `OFF` |
| `COVERAGE` | Enable coverage tracing when testing | `OFF` |

## Rust FFT Extension (`genalyzer._fourier_rs`)

A Rust-based FFT implementation is available as an optional Python extension module
(`genalyzer._fourier_rs`). It is optimised for repeated, concurrent calls and provides
a `FourierAnalyzer` class as well as stateless free functions (`forward_fft`,
`inverse_fft`, `power_spectrum`, `frequencies`).

### Additional Dependencies

- [Rust toolchain](https://rustup.rs/) (stable, 1.70 or later)
- [maturin](https://github.com/PyO3/maturin) Python build tool

```bash
pip install maturin
```

### Build (development install)

```bash
cd rust/fourier
maturin develop --features python
```

This compiles the extension and installs it into the active Python environment so
that `import genalyzer._fourier_rs` works immediately.

### Build (release wheel)

```bash
cd rust/fourier
maturin build --release --features python
pip install target/wheels/genalyzer_fourier_rs-*.whl
```

### Normalization convention

The forward FFT is unnormalized. The inverse FFT divides by `N`, matching the
`numpy.fft` convention used by the existing Python implementation.


