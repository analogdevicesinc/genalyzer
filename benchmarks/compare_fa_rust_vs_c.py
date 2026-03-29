#!/usr/bin/env python3
import argparse
import json
import os
import platform
import statistics
import subprocess
import time
from datetime import datetime, timezone
from pathlib import Path


def run_and_time(cmd, cwd=None, env=None):
    t0 = time.perf_counter()
    proc = subprocess.run(
        cmd, cwd=cwd, env=env, check=True, capture_output=True, text=True
    )
    elapsed = time.perf_counter() - t0
    return elapsed, proc.stdout.strip(), proc.stderr.strip()


def median(xs):
    return statistics.median(xs) if xs else float("nan")


def parse_json_line(out):
    last = out.strip().splitlines()[-1]
    return json.loads(last)


def main():
    parser = argparse.ArgumentParser(description="Compare Rust and C Fourier-analysis performance")
    parser.add_argument("--input", default="tests/test_vectors/test_Pluto_DDS_data_1658159639196.json")
    parser.add_argument("--runs", type=int, default=7)
    parser.add_argument("--iters", type=int, default=200)
    parser.add_argument("--warmup", type=int, default=20)
    parser.add_argument("--build-dir", default="build")
    args = parser.parse_args()

    repo = Path(__file__).resolve().parents[1]
    build_dir = repo / args.build_dir
    input_path = str((repo / args.input).resolve())

    c_end_to_end_bin = build_dir / "examples" / "do_fa_pluto_example_all_results"
    c_compute_bin = build_dir / "examples" / "do_fa_pluto_example_all_results_bench"

    rust_build_env = os.environ.copy()
    rust_build_env["PATH"] = f"{Path.home() / '.cargo' / 'bin'}:{rust_build_env.get('PATH', '')}"
    subprocess.run(
        ["cargo", "build", "--release", "-p", "genalyzer-fa", "--examples"],
        cwd=repo,
        env=rust_build_env,
        check=True,
    )

    rust_end_to_end_bin = repo / "target" / "release" / "examples" / "do_fa_pluto_example_all_results"
    rust_compute_bin = repo / "target" / "release" / "examples" / "do_fa_pluto_example_all_results_bench"
    rust_end_to_end_cmd = [str(rust_end_to_end_bin), input_path]
    rust_compute_cmd = [
        str(rust_compute_bin),
        "--input",
        input_path,
        "--iters",
        str(args.iters),
        "--warmup",
        str(args.warmup),
    ]

    c_env = os.environ.copy()
    ld_parts = [str(build_dir / "src"), str(build_dir / "bindings" / "c" / "src")]
    if c_env.get("LD_LIBRARY_PATH"):
        ld_parts.append(c_env["LD_LIBRARY_PATH"])
    c_env["LD_LIBRARY_PATH"] = ":".join(ld_parts)

    c_end_to_end_times = []
    rust_end_to_end_times = []

    for _ in range(args.runs):
        elapsed, _, _ = run_and_time([str(c_end_to_end_bin)], cwd=repo / "examples", env=c_env)
        c_end_to_end_times.append(elapsed)
        elapsed, _, _ = run_and_time(rust_end_to_end_cmd, cwd=repo)
        rust_end_to_end_times.append(elapsed)

    c_compute_times = []
    rust_compute_times = []
    c_compute_payload = None
    rust_compute_payload = None

    for _ in range(args.runs):
        _, out, _ = run_and_time(
            [
                str(c_compute_bin),
                "--input",
                input_path,
                "--iters",
                str(args.iters),
                "--warmup",
                str(args.warmup),
            ]
            ,
            env=c_env,
        )
        payload = parse_json_line(out)
        c_compute_payload = payload
        c_compute_times.append(float(payload["elapsed_sec"]))

        _, out, _ = run_and_time(rust_compute_cmd, cwd=repo)
        payload = parse_json_line(out)
        rust_compute_payload = payload
        rust_compute_times.append(float(payload["elapsed_sec"]))

    c_e2e = median(c_end_to_end_times)
    r_e2e = median(rust_end_to_end_times)
    c_comp = median(c_compute_times)
    r_comp = median(rust_compute_times)

    result = {
        "timestamp_utc": datetime.now(timezone.utc).isoformat(),
        "platform": {
            "system": platform.system(),
            "release": platform.release(),
            "machine": platform.machine(),
            "python": platform.python_version(),
        },
        "config": {
            "input": input_path,
            "runs": args.runs,
            "iters": args.iters,
            "warmup": args.warmup,
        },
        "end_to_end": {
            "c_median_sec": c_e2e,
            "rust_median_sec": r_e2e,
            "rust_over_c": r_e2e / c_e2e if c_e2e > 0 else float("inf"),
            "c_over_rust": c_e2e / r_e2e if r_e2e > 0 else float("inf"),
        },
        "compute_only": {
            "c_median_sec": c_comp,
            "rust_median_sec": r_comp,
            "rust_over_c": r_comp / c_comp if c_comp > 0 else float("inf"),
            "c_over_rust": c_comp / r_comp if r_comp > 0 else float("inf"),
            "c_last": c_compute_payload,
            "rust_last": rust_compute_payload,
        },
    }

    results_dir = repo / "benchmarks" / "results"
    results_dir.mkdir(parents=True, exist_ok=True)
    out_file = results_dir / f"fa_rust_vs_c_{datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')}.json"
    out_file.write_text(json.dumps(result, indent=2))

    print("Performance Summary")
    print(f"  End-to-end median  : C={c_e2e:.6f}s  Rust={r_e2e:.6f}s  Rust/C={result['end_to_end']['rust_over_c']:.3f}x")
    print(f"  Compute-only median: C={c_comp:.6f}s  Rust={r_comp:.6f}s  Rust/C={result['compute_only']['rust_over_c']:.3f}x")
    print(f"Saved: {out_file}")


if __name__ == "__main__":
    main()
