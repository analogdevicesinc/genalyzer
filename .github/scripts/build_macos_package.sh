#!/usr/bin/env bash
set -euo pipefail

distro=macos
build_dir="build-package-${distro}"
artifact_dir="artifacts/${distro}"
stage_dir="${RUNNER_TEMP}/genalyzer-package-root"
rm -rf "$build_dir" "$artifact_dir"

cmake -S . -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DGENALYZER_BUILD_FRAMEWORKS=OFF \
    -DGENALYZER_NATIVE_OPTIMIZATIONS=OFF \
    -DGENALYZER_PACKAGE_DISTRO="$distro"
cmake --build "$build_dir" --parallel 2
package_version=$(sed -n 's/^set(CPACK_PACKAGE_VERSION "\(.*\)")/\1/p' \
    "$build_dir/CPackConfig.cmake")
[[ -n "$package_version" ]]

rm -rf "$stage_dir"
DESTDIR="$stage_dir" cmake --install "$build_dir"
mkdir -p "$artifact_dir"
package="$artifact_dir/libgenalyzer-${package_version}-macos-$(uname -m).pkg"
/usr/bin/pkgbuild \
    --root "$stage_dir" \
    --scripts .github/scripts/macos_package_scripts \
    --identifier com.analogdevices.genalyzer \
    --version "$package_version" \
    --install-location / \
    "$package"
cp .github/scripts/macos_package_README.txt "$artifact_dir/README.txt"

pkgutil --check-signature "$package" || true
rm -rf "$RUNNER_TEMP/genalyzer-package"
pkgutil --expand "$package" "$RUNNER_TEMP/genalyzer-package"
find "$RUNNER_TEMP/genalyzer-package" -type f -print
test -x "$RUNNER_TEMP/genalyzer-package/Scripts/preinstall"
sudo installer -pkg "$package" -target /

package_id=$(pkgutil --pkgs | grep -m 1 '^com\.analogdevices\.genalyzer')
test -n "$package_id"
pkgutil --files "$package_id"
test -f /usr/local/include/cgenalyzer.h
test -f /usr/local/share/licenses/genalyzer/LICENSE
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig pkg-config --modversion libgenalyzer
dylib_dependencies=$(otool -L /usr/local/lib/libgenalyzer.dylib)
printf '%s\n' "$dylib_dependencies"
grep -Fq '/opt/homebrew/opt/fftw/lib/libfftw3.3.dylib' \
    <<<"$dylib_dependencies"
if grep -Eq 'libfftw3(f|l|_threads|_omp)' <<<"$dylib_dependencies"; then
    printf 'unexpected FFTW runtime dependency\n' >&2
    exit 1
fi

smoke_dir=$(mktemp -d)
trap 'rm -rf "$smoke_dir"' EXIT
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig \
    cc .github/scripts/package_smoke.c \
    -DEXPECTED_VERSION="\"$package_version\"" \
    $(PKG_CONFIG_PATH=/usr/local/lib/pkgconfig pkg-config --cflags --libs libgenalyzer) \
    -o "$smoke_dir/package-smoke"
(cd "$smoke_dir" && ./package-smoke)
file "$package"