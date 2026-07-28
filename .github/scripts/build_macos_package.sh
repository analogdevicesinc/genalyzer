#!/usr/bin/env bash
set -euo pipefail

distro=macos
build_dir="build-package-${distro}"
artifact_dir="artifacts/${distro}"
rm -rf "$build_dir" "$artifact_dir"

cmake -S . -B "$build_dir" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DGENALYZER_BUILD_FRAMEWORKS=OFF \
    -DGENALYZER_NATIVE_OPTIMIZATIONS=OFF \
    -DGENALYZER_PACKAGE_DISTRO="$distro"
cmake --build "$build_dir" --parallel 2
cpack --config "$build_dir/CPackConfig.cmake" -G productbuild \
    -B "$artifact_dir"
package_version=$(sed -n 's/^set(CPACK_PACKAGE_VERSION "\(.*\)")/\1/p' \
    "$build_dir/CPackConfig.cmake")
[[ -n "$package_version" ]]

shopt -s nullglob
packages=("$artifact_dir"/*.pkg)
[[ ${#packages[@]} -eq 1 ]]
pkgutil --check-signature "${packages[0]}" || true
rm -rf "$RUNNER_TEMP/genalyzer-package"
pkgutil --expand "${packages[0]}" "$RUNNER_TEMP/genalyzer-package"
find "$RUNNER_TEMP/genalyzer-package" -maxdepth 3 -type f -print
sudo installer -pkg "${packages[0]}" -target /

package_id=$(pkgutil --pkgs | grep -m 1 '^com\.analogdevices\.genalyzer')
test -n "$package_id"
pkgutil --files "$package_id"
test -f /usr/local/include/cgenalyzer.h
test -f /usr/local/share/licenses/genalyzer/LICENSE
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig pkg-config --modversion libgenalyzer

smoke_dir=$(mktemp -d)
trap 'rm -rf "$smoke_dir"' EXIT
PKG_CONFIG_PATH=/usr/local/lib/pkgconfig \
    cc .github/scripts/package_smoke.c \
    -DEXPECTED_VERSION="\"$package_version\"" \
    $(PKG_CONFIG_PATH=/usr/local/lib/pkgconfig pkg-config --cflags --libs libgenalyzer) \
    -o "$smoke_dir/package-smoke"
(cd "$smoke_dir" && DYLD_LIBRARY_PATH=/usr/local/lib ./package-smoke)
file "${packages[0]}"