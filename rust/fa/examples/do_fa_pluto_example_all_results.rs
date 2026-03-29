use genalyzer_fa::{
    configure_fa_auto, fft_analysis, FftzConfig, FourierAnalyzerFast, FreqAxisType, Window,
};
use serde::Deserialize;
use std::env;
use std::fs;

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

fn main() -> Result<(), String> {
    let default_file = "tests/test_vectors/test_Pluto_DDS_data_1658159639196.json".to_string();
    let json_file = env::args().nth(1).unwrap_or(default_file);

    let data = load_input(&json_file)?;
    let cfg = FftzConfig::new(
        data.npts,
        data.qres,
        data.navg,
        data.nfft,
        map_window(data.win),
    )?;
    let analyzer = FourierAnalyzerFast::new(&cfg);
    let fft = analyzer.fftz_interleaved(&data.test_vec_i, &data.test_vec_q, &cfg)?;

    let fa_cfg = configure_fa_auto(data.fs, 120)?;
    let results = fft_analysis(&fft, data.nfft, &fa_cfg, FreqAxisType::DcCenter)?;

    println!("\nAll Fourier Analysis Results:");
    for (idx, (key, value)) in results.flattened().iter().enumerate() {
        println!("{idx:4}{key:20}{value:20.6}");
    }
    Ok(())
}
