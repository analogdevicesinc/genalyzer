#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 ]]; then
    echo "usage: $0 <DEB|RPM> <distribution-label>" >&2
    exit 2
fi

generator=$1
distro=$2
case "$generator" in
    DEB)
        export DEBIAN_FRONTEND=noninteractive
        apt-get update
        apt-get install -y --no-install-recommends \
            build-essential cmake file libfftw3-dev ninja-build pkg-config
        ;;
    RPM)
        dnf install -y \
            cmake file fftw-devel gcc gcc-c++ make ninja-build pkgconf-pkg-config rpm-build
        ;;
    *)
        echo "unsupported CPack generator: $generator" >&2
        exit 2
        ;;
esac

build_dir="build-package-${distro}"
artifact_dir="artifacts/${distro}"
rm -rf "$build_dir" "$artifact_dir"

cmake -S . -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DGENALYZER_BUILD_FRAMEWORKS=OFF \
    -DGENALYZER_NATIVE_OPTIMIZATIONS=OFF \
    -DGENALYZER_PACKAGE_DISTRO="$distro"
cmake --build "$build_dir" --parallel 2
cpack --config "$build_dir/CPackConfig.cmake" -G "$generator" \
    -B "$artifact_dir"
package_version=$(sed -n 's/^set(CPACK_PACKAGE_VERSION "\(.*\)")/\1/p' \
    "$build_dir/CPackConfig.cmake")
[[ -n "$package_version" ]]

shopt -s nullglob
case "$generator" in
    DEB)
        packages=("$artifact_dir"/*.deb)
        [[ ${#packages[@]} -eq 1 ]]
        dpkg-deb --info "${packages[0]}"
        dpkg-deb --contents "${packages[0]}"
        apt-get install -y "./${packages[0]}"
        dpkg-query -L libgenalyzer
        ;;
    RPM)
        packages=("$artifact_dir"/*.rpm)
        [[ ${#packages[@]} -eq 1 ]]
        rpm -qip "${packages[0]}"
        rpm -qlp "${packages[0]}"
        dnf install -y "${packages[0]}"
        rpm -ql libgenalyzer
        ;;
esac

ldconfig
test -f /usr/include/cgenalyzer.h
test -f /usr/share/licenses/genalyzer/LICENSE
pkg-config --modversion libgenalyzer

smoke_dir=$(mktemp -d)
trap 'rm -rf "$smoke_dir"' EXIT
cc .github/scripts/package_smoke.c \
    -DEXPECTED_VERSION="\"$package_version\"" \
    $(pkg-config --cflags --libs libgenalyzer) \
    -o "$smoke_dir/package-smoke"
(cd "$smoke_dir" && ./package-smoke)
file "${packages[0]}"