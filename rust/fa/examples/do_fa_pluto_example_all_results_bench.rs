use genalyzer_fa::{
    configure_fa_auto, fft_analysis, FftzConfig, FourierAnalyzerFast, FreqAxisType, Window,
};
use serde::Deserialize;
use std::env;
use std::fs;
use std::time::Instant;

#[derive(Debug, Deserialize)]
struct PlutoInput {
    qres: i32,
    npts: usize,
    navg: usize,
    nfft: usize,
    fs: f64,
    win: u64,
    test_vec_i: Vec<i32>,
    test_vec_q: Vec<i32>,
}

fn map_window(win: u64) -> Window {
    match win {
        1 => Window::BlackmanHarris,
        2 => Window::Hann,
        _ => Window::NoWindow,
    }
}

fn load_input(path: &str) -> Result<PlutoInput, String> {
    let content = fs::read_to_string(path).map_err(|e| format!("failed to read {path}: {e}"))?;
    serde_json::from_str(&content).map_err(|e| format!("invalid JSON in {path}: {e}"))
}

fn parse_arg(args: &[String], key: &str, default: usize) -> usize {
    args.windows(2)
        .find_map(|w| {
            if w[0] == key {
                w[1].parse::<usize>().ok()
            } else {
                None
            }
        })
        .unwrap_or(default)
}

fn parse_path(args: &[String]) -> String {
    args.windows(2)
        .find_map(|w| {
            if w[0] == "--input" {
                Some(w[1].clone())
            } else {
                None
            }
        })
        .unwrap_or_else(|| "tests/test_vectors/test_Pluto_DDS_data_1658159639196.json".to_string())
}

fn main() -> Result<(), String> {
    let args: Vec<String> = env::args().collect();
    let iters = parse_arg(&args, "--iters", 200);
    let warmup = parse_arg(&args, "--warmup", 20);
    let input = parse_path(&args);

    let data = load_input(&input)?;
    let cfg = FftzConfig::new(
        data.npts,
        data.qres,
        data.navg,
        data.nfft,
        map_window(data.win),
    )?;
    let analyzer = FourierAnalyzerFast::new(&cfg);
    let fa_cfg = configure_fa_auto(data.fs, 120)?;

    for _ in 0..warmup {
        let fft = analyzer.fftz_interleaved(&data.test_vec_i, &data.test_vec_q, &cfg)?;
        let _ = fft_analysis(&fft, data.nfft, &fa_cfg, FreqAxisType::DcCenter)?;
    }

    let t0 = Instant::now();
    let mut checksum = 0.0f64;
    for _ in 0..iters {
        let fft = analyzer.fftz_interleaved(&data.test_vec_i, &data.test_vec_q, &cfg)?;
        let r = fft_analysis(&fft, data.nfft, &fa_cfg, FreqAxisType::DcCenter)?;
        checksum += r.checksum();
    }
    let elapsed = t0.elapsed().as_secs_f64();

    println!("{{\"impl\":\"rust\",\"iters\":{iters},\"warmup\":{warmup},\"elapsed_sec\":{elapsed:.9},\"checksum\":{checksum:.9}}}");
    Ok(())
}
