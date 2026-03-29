// Copyright (C) 2026 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later

use num_complex::Complex;
use rustfft::{Fft, FftPlanner};
use std::collections::{BTreeMap, HashSet};
use std::f64::consts::PI;
use std::sync::Arc;

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum Window {
    BlackmanHarris,
    Hann,
    NoWindow,
}

#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum FreqAxisType {
    DcCenter,
    DcLeft,
    Real,
}

#[derive(Clone, Debug)]
pub struct FftzConfig {
    pub npts: usize,
    pub qres: i32,
    pub fft_navg: usize,
    pub nfft: usize,
    pub win: Window,
}

impl FftzConfig {
    pub fn new(
        npts: usize,
        qres: i32,
        fft_navg: usize,
        nfft: usize,
        win: Window,
    ) -> Result<Self, String> {
        if npts != fft_navg.saturating_mul(nfft) {
            return Err("npts must equal fft_navg * nfft".to_string());
        }
        if qres <= 0 || qres > 32 {
            return Err("qres must be in [1, 32]".to_string());
        }
        if nfft == 0 {
            return Err("nfft must be > 0".to_string());
        }
        Ok(Self {
            npts,
            qres,
            fft_navg,
            nfft,
            win,
        })
    }
}

#[derive(Clone, Debug)]
pub struct FaAutoConfig {
    pub sample_rate: f64,
    pub ssb_width: usize,
    pub ssb_rest: usize,
    pub max_harm_order: usize,
    pub axis_type: FreqAxisType,
}

pub fn configure_fa_auto(sample_rate: f64, ssb_width: u8) -> Result<FaAutoConfig, String> {
    if sample_rate <= 0.0 {
        return Err("sample_rate must be > 0".to_string());
    }
    Ok(FaAutoConfig {
        sample_rate,
        ssb_width: ssb_width as usize,
        ssb_rest: 0,
        max_harm_order: 3,
        axis_type: FreqAxisType::DcCenter,
    })
}

#[derive(Clone, Debug, Default)]
pub struct AnalysisResults {
    pub global: BTreeMap<String, f64>,
    pub tone_results: BTreeMap<String, BTreeMap<String, f64>>,
}

impl AnalysisResults {
    pub fn flattened(&self) -> Vec<(String, f64)> {
        let mut out = Vec::new();
        for key in FA_RESULT_KEYS {
            if let Some(value) = self.global.get(*key) {
                out.push(((*key).to_string(), *value));
            }
        }
        for tone_key in ["A", "dc", "2A", "3A", "wo1"] {
            if let Some(tmap) = self.tone_results.get(tone_key) {
                for tk in FA_TONE_RESULT_KEYS {
                    if let Some(v) = tmap.get(*tk) {
                        out.push((format!("{tone_key}:{tk}"), *v));
                    }
                }
            }
        }
        out
    }

    pub fn checksum(&self) -> f64 {
        self.flattened().into_iter().map(|(_, v)| v).sum()
    }
}

pub struct FourierAnalyzerFast {
    nfft: usize,
    plan: Arc<dyn Fft<f64>>,
    scratch_len: usize,
    window: Vec<f64>,
}

impl FourierAnalyzerFast {
    pub fn new(cfg: &FftzConfig) -> Self {
        let mut planner = FftPlanner::<f64>::new();
        let plan = planner.plan_fft_forward(cfg.nfft);
        let scratch_len = plan.get_inplace_scratch_len();
        Self {
            nfft: cfg.nfft,
            plan,
            scratch_len,
            window: build_window(cfg.nfft, cfg.win),
        }
    }

    pub fn fftz_i32(
        &self,
        i_data: &[i32],
        q_data: &[i32],
        cfg: &FftzConfig,
    ) -> Result<Vec<Complex<f64>>, String> {
        if i_data.len() != cfg.npts || q_data.len() != cfg.npts {
            return Err("input lengths must match cfg.npts".to_string());
        }
        if cfg.nfft != self.nfft {
            return Err("cfg.nfft differs from analyzer plan nfft".to_string());
        }

        let scale = 1.0f64 / ((1_i64 << (cfg.qres.saturating_sub(1) as u32)) as f64);
        let mut accum = vec![Complex::new(0.0, 0.0); cfg.nfft];

        for avg_idx in 0..cfg.fft_navg {
            let base = avg_idx * cfg.nfft;
            let mut buf = vec![Complex::new(0.0, 0.0); cfg.nfft];
            for k in 0..cfg.nfft {
                let re = i_data[base + k] as f64 * scale * self.window[k];
                let im = q_data[base + k] as f64 * scale * self.window[k];
                buf[k] = Complex::new(re, im);
            }
            let mut scratch = vec![Complex::new(0.0, 0.0); self.scratch_len];
            self.plan.process_with_scratch(&mut buf, &mut scratch);
            for (dst, src) in accum.iter_mut().zip(buf.iter()) {
                *dst += *src;
            }
        }

        let norm = 1.0 / (cfg.fft_navg as f64 * cfg.nfft as f64);
        for x in &mut accum {
            *x *= norm;
        }
        Ok(accum)
    }

    pub fn fftz_interleaved(
        &self,
        i_data: &[i32],
        q_data: &[i32],
        cfg: &FftzConfig,
    ) -> Result<Vec<f64>, String> {
        let c = self.fftz_i32(i_data, q_data, cfg)?;
        Ok(to_interleaved(&c))
    }
}

pub fn fft_analysis(
    fft_ilv: &[f64],
    nfft: usize,
    cfg: &FaAutoConfig,
    axis_type: FreqAxisType,
) -> Result<AnalysisResults, String> {
    if nfft == 0 {
        return Err("nfft must be > 0".to_string());
    }
    if fft_ilv.len() != 2 * nfft {
        return Err("fft_ilv must be 2*nfft interleaved Re/Im values".to_string());
    }

    let spectrum = from_interleaved(fft_ilv);
    let magsq: Vec<f64> = spectrum.iter().map(|x| x.norm_sqr()).collect();

    let dc_idx = 0usize;
    let carrier_idx = magsq
        .iter()
        .enumerate()
        .skip(1)
        .max_by(|a, b| a.1.partial_cmp(b.1).unwrap_or(std::cmp::Ordering::Equal))
        .map(|(idx, _)| idx)
        .unwrap_or(0);

    let ssb = cfg.ssb_width;
    let mut sig_bins = bin_span(carrier_idx, ssb, nfft);
    if sig_bins.is_empty() {
        sig_bins.push(carrier_idx);
    }

    let dc_bins = bin_span(dc_idx, ssb, nfft);
    let mut hd_bins: HashSet<usize> = HashSet::new();
    let mut tone_infos = Vec::new();

    tone_infos.push(tone_metrics(
        "dc",
        0,
        dc_idx,
        &dc_bins,
        &spectrum,
        &magsq,
        cfg.sample_rate,
        nfft,
        0,
    ));
    tone_infos.push(tone_metrics(
        "A",
        1,
        carrier_idx,
        &sig_bins,
        &spectrum,
        &magsq,
        cfg.sample_rate,
        nfft,
        1,
    ));

    for h in 2..=cfg.max_harm_order {
        let idx = (carrier_idx * h) % nfft;
        let bins = bin_span(idx, ssb, nfft);
        for b in &bins {
            hd_bins.insert(*b);
        }
        let key = format!("{h}A");
        tone_infos.push(tone_metrics(
            &key,
            h,
            idx,
            &bins,
            &spectrum,
            &magsq,
            cfg.sample_rate,
            nfft,
            2,
        ));
    }

    let mut excluded: HashSet<usize> = dc_bins.iter().copied().collect();
    excluded.extend(sig_bins.iter().copied());
    excluded.extend(hd_bins.iter().copied());

    let mut wo_idx = 0usize;
    let mut wo_val = f64::NEG_INFINITY;
    for (i, v) in magsq.iter().enumerate() {
        if !excluded.contains(&i) && *v > wo_val {
            wo_val = *v;
            wo_idx = i;
        }
    }
    let wo_bins = bin_span(wo_idx, cfg.ssb_rest.max(1), nfft);
    tone_infos.push(tone_metrics(
        "wo1",
        1,
        wo_idx,
        &wo_bins,
        &spectrum,
        &magsq,
        cfg.sample_rate,
        nfft,
        8,
    ));

    let all_sum: f64 = magsq.iter().sum();
    let signal_ss = sum_bins(&magsq, &sig_bins);
    let dc_ss = sum_bins(&magsq, &dc_bins);
    let hd_ss: f64 = hd_bins.iter().map(|idx| magsq[*idx]).sum();
    let wo_ss = sum_bins(&magsq, &wo_bins);

    let dist_ss = dc_ss + hd_ss + wo_ss;
    let noise_ss = (all_sum - signal_ss - dist_ss).max(0.0);
    let nad_ss = noise_ss + dist_ss;

    let mut r = AnalysisResults::default();
    let fbin = cfg.sample_rate / nfft as f64;

    r.global.insert("analysistype".to_string(), 1.0);
    r.global.insert("signaltype".to_string(), 1.0);
    r.global.insert("nfft".to_string(), nfft as f64);
    r.global.insert("datasize".to_string(), (2 * nfft) as f64);
    r.global.insert("fbin".to_string(), fbin);
    r.global.insert("fdata".to_string(), cfg.sample_rate);
    r.global.insert("fsample".to_string(), cfg.sample_rate);
    r.global.insert("fshift".to_string(), 0.0);
    r.global.insert("fsnr".to_string(), ratio_db(1.0, noise_ss));
    r.global
        .insert("snr".to_string(), ratio_db(signal_ss, noise_ss));
    r.global
        .insert("sinad".to_string(), ratio_db(signal_ss, nad_ss));
    r.global
        .insert("sfdr".to_string(), ratio_db(signal_ss, wo_ss.max(1e-30)));
    r.global.insert(
        "abn".to_string(),
        ratio_db(signal_ss + nad_ss, all_sum.max(1e-30)),
    );
    r.global
        .insert("nsd".to_string(), ratio_db(noise_ss / nfft as f64, 1.0));
    r.global
        .insert("carrierindex".to_string(), carrier_idx as f64);
    r.global.insert("maxspurindex".to_string(), wo_idx as f64);
    r.global.insert("ab_width".to_string(), cfg.sample_rate);
    r.global.insert("ab_i1".to_string(), 0.0);
    r.global
        .insert("ab_i2".to_string(), (nfft.saturating_sub(1)) as f64);
    r.global.insert("ab_nbins".to_string(), nfft as f64);
    r.global.insert("ab_rss".to_string(), all_sum.sqrt());
    r.global
        .insert("signal_nbins".to_string(), sig_bins.len() as f64);
    r.global.insert("signal_rss".to_string(), signal_ss.sqrt());
    r.global.insert("clk_nbins".to_string(), 0.0);
    r.global.insert("clk_rss".to_string(), 0.0);
    r.global
        .insert("hd_nbins".to_string(), hd_bins.len() as f64);
    r.global.insert("hd_rss".to_string(), hd_ss.sqrt());
    r.global.insert("ilos_nbins".to_string(), 0.0);
    r.global.insert("ilos_rss".to_string(), 0.0);
    r.global.insert("ilgt_nbins".to_string(), 0.0);
    r.global.insert("ilgt_rss".to_string(), 0.0);
    r.global.insert("imd_nbins".to_string(), 0.0);
    r.global.insert("imd_rss".to_string(), 0.0);
    r.global.insert("userdist_nbins".to_string(), 0.0);
    r.global.insert("userdist_rss".to_string(), 0.0);
    r.global
        .insert("thd_nbins".to_string(), hd_bins.len() as f64);
    r.global.insert("thd_rss".to_string(), hd_ss.sqrt());
    r.global.insert("ilv_nbins".to_string(), 0.0);
    r.global.insert("ilv_rss".to_string(), 0.0);
    r.global.insert(
        "dist_nbins".to_string(),
        (dc_bins.len() + hd_bins.len() + wo_bins.len()) as f64,
    );
    r.global.insert("dist_rss".to_string(), dist_ss.sqrt());
    r.global
        .insert("noise_nbins".to_string(), (nfft - sig_bins.len()) as f64);
    r.global.insert("noise_rss".to_string(), noise_ss.sqrt());
    r.global
        .insert("nad_nbins".to_string(), (nfft - sig_bins.len()) as f64);
    r.global.insert("nad_rss".to_string(), nad_ss.sqrt());

    let carrier_mag = tone_infos
        .iter()
        .find(|x| x.key == "A")
        .map(|x| x.mag)
        .unwrap_or(1e-30)
        .max(1e-30);

    for t in tone_infos {
        let mut m = BTreeMap::new();
        m.insert("orderindex".to_string(), t.order_index as f64);
        m.insert("tag".to_string(), t.tag as f64);
        m.insert("freq".to_string(), t.freq);
        m.insert("ffinal".to_string(), t.freq);
        m.insert("fwavg".to_string(), t.freq);
        m.insert("i1".to_string(), t.i1 as f64);
        m.insert("i2".to_string(), t.i2 as f64);
        m.insert("nbins".to_string(), t.nbins as f64);
        m.insert("inband".to_string(), 1.0);
        m.insert("mag".to_string(), t.mag);
        m.insert("mag_dbfs".to_string(), power_db(t.mag * t.mag));
        m.insert(
            "mag_dbc".to_string(),
            ratio_db(t.mag * t.mag, carrier_mag * carrier_mag),
        );
        m.insert("phase".to_string(), t.phase);
        m.insert("phase_c".to_string(), t.phase);
        r.tone_results.insert(t.key.to_string(), m);
    }

    // Ensure all global keys exist to preserve key parity surface.
    for key in FA_RESULT_KEYS {
        r.global.entry((*key).to_string()).or_insert(0.0);
    }

    // Axis type currently reserved for future detailed parity behavior.
    let _ = axis_type;

    Ok(r)
}

#[derive(Clone, Debug)]
struct ToneMetrics {
    key: String,
    order_index: usize,
    tag: i32,
    freq: f64,
    i1: usize,
    i2: usize,
    nbins: usize,
    mag: f64,
    phase: f64,
}

fn tone_metrics(
    key: &str,
    order_index: usize,
    center_idx: usize,
    bins: &[usize],
    spectrum: &[Complex<f64>],
    magsq: &[f64],
    fs: f64,
    nfft: usize,
    tag: i32,
) -> ToneMetrics {
    let mag2 = sum_bins(magsq, bins);
    let phase = spectrum[center_idx].arg();
    let i1 = *bins.first().unwrap_or(&center_idx);
    let i2 = *bins.last().unwrap_or(&center_idx);
    ToneMetrics {
        key: key.to_string(),
        order_index,
        tag,
        freq: center_idx as f64 * fs / nfft as f64,
        i1,
        i2,
        nbins: bins.len(),
        mag: mag2.sqrt(),
        phase,
    }
}

fn ratio_db(num: f64, den: f64) -> f64 {
    if num <= 0.0 || den <= 0.0 {
        return f64::NEG_INFINITY;
    }
    10.0 * (num / den).log10()
}

fn power_db(v: f64) -> f64 {
    if v <= 0.0 {
        return f64::NEG_INFINITY;
    }
    10.0 * v.log10()
}

fn sum_bins(magsq: &[f64], bins: &[usize]) -> f64 {
    bins.iter().map(|idx| magsq[*idx]).sum()
}

fn bin_span(center: usize, ssb: usize, nfft: usize) -> Vec<usize> {
    if nfft == 0 {
        return Vec::new();
    }
    let mut out = Vec::with_capacity(2 * ssb + 1);
    for d in 0..=ssb {
        let lo = (center + nfft - d % nfft) % nfft;
        let hi = (center + d) % nfft;
        if !out.contains(&lo) {
            out.push(lo);
        }
        if !out.contains(&hi) {
            out.push(hi);
        }
    }
    out.sort_unstable();
    out
}

fn build_window(n: usize, w: Window) -> Vec<f64> {
    match w {
        Window::NoWindow => vec![1.0; n],
        Window::Hann => (0..n)
            .map(|k| 0.5 - 0.5 * (2.0 * PI * k as f64 / (n as f64)).cos())
            .collect(),
        Window::BlackmanHarris => {
            let a0 = 0.35875;
            let a1 = 0.48829;
            let a2 = 0.14128;
            let a3 = 0.01168;
            (0..n)
                .map(|k| {
                    let x = 2.0 * PI * k as f64 / (n as f64);
                    a0 - a1 * x.cos() + a2 * (2.0 * x).cos() - a3 * (3.0 * x).cos()
                })
                .collect()
        }
    }
}

pub fn to_interleaved(spectrum: &[Complex<f64>]) -> Vec<f64> {
    let mut out = Vec::with_capacity(2 * spectrum.len());
    for c in spectrum {
        out.push(c.re);
        out.push(c.im);
    }
    out
}

pub fn from_interleaved(fft_ilv: &[f64]) -> Vec<Complex<f64>> {
    fft_ilv
        .chunks_exact(2)
        .map(|xy| Complex::new(xy[0], xy[1]))
        .collect()
}

pub const FA_RESULT_KEYS: &[&str] = &[
    "analysistype",
    "signaltype",
    "nfft",
    "datasize",
    "fbin",
    "fdata",
    "fsample",
    "fshift",
    "fsnr",
    "snr",
    "sinad",
    "sfdr",
    "abn",
    "nsd",
    "carrierindex",
    "maxspurindex",
    "ab_width",
    "ab_i1",
    "ab_i2",
    "ab_nbins",
    "ab_rss",
    "signal_nbins",
    "signal_rss",
    "clk_nbins",
    "clk_rss",
    "hd_nbins",
    "hd_rss",
    "ilos_nbins",
    "ilos_rss",
    "ilgt_nbins",
    "ilgt_rss",
    "imd_nbins",
    "imd_rss",
    "userdist_nbins",
    "userdist_rss",
    "thd_nbins",
    "thd_rss",
    "ilv_nbins",
    "ilv_rss",
    "dist_nbins",
    "dist_rss",
    "noise_nbins",
    "noise_rss",
    "nad_nbins",
    "nad_rss",
];

pub const FA_TONE_RESULT_KEYS: &[&str] = &[
    "orderindex",
    "tag",
    "freq",
    "ffinal",
    "fwavg",
    "i1",
    "i2",
    "nbins",
    "inband",
    "mag",
    "mag_dbfs",
    "mag_dbc",
    "phase",
    "phase_c",
];

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn flattened_contains_global_keys() {
        let cfg = FftzConfig::new(8, 12, 1, 8, Window::NoWindow).unwrap();
        let fa = configure_fa_auto(1_000_000.0, 4).unwrap();
        let an = FourierAnalyzerFast::new(&cfg);
        let i = vec![0; 8];
        let q = vec![0; 8];
        let fft = an.fftz_interleaved(&i, &q, &cfg).unwrap();
        let r = fft_analysis(&fft, cfg.nfft, &fa, FreqAxisType::DcCenter).unwrap();
        assert_eq!(r.global.len(), FA_RESULT_KEYS.len());
        let flat = r.flattened();
        assert!(flat.iter().any(|(k, _)| k == "snr"));
    }
}
