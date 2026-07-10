#!/usr/bin/env bash
set -euo pipefail

version="3.3.10"
archive="fftw-${version}.tar.gz"
url="https://fftw.org/${archive}"
sha256="56c932549852cddcfafdab3820b0200c7742675be92179e59e6215b340e26467"
work_dir="$(mktemp -d)"
trap 'rm -rf "${work_dir}"' EXIT

curl --fail --location --retry 3 "${url}" --output "${work_dir}/${archive}"
echo "${sha256}  ${work_dir}/${archive}" | shasum --algorithm 256 --check

tar -xzf "${work_dir}/${archive}" -C "${work_dir}"
cd "${work_dir}/fftw-${version}"

export CFLAGS="${CFLAGS:-} -mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"
export CXXFLAGS="${CXXFLAGS:-} -mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"
export LDFLAGS="${LDFLAGS:-} -mmacosx-version-min=${MACOSX_DEPLOYMENT_TARGET}"

./configure \
    --prefix=/usr/local \
    --enable-shared \
    --disable-static \
    --enable-threads \
    --disable-fortran
make -j"$(sysctl -n hw.logicalcpu)"
sudo make install
