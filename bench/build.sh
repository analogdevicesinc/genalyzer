#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
# Ensure the real system assembler (/usr/bin/as) is found before any user-local
# wrapper that might shadow it (e.g. ~/.local/bin/as used as an agent launcher).
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin${PATH:+:$PATH}"
g++ -O3 -mavx -std=c++17 -pthread -c bh_window_bench.cpp -o bh_window_bench.o
g++ -c bh_window_asm.S -o bh_window_asm.o
g++ bh_window_bench.o bh_window_asm.o -o bh_window_bench -pthread
echo "built ./bench/bh_window_bench"
