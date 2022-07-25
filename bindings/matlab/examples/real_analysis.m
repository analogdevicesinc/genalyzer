genalyzer.load()
gn = genalyzer;
fprintf('Data Converter Analysis Library\nVersion: %s\n', gn.version())

%
% Setup
%
doplot      = true;
navg        = 2;                                % FFT averaging number
nfft        = 1024 * 256;                       % FFT size
fs          = 1e9;                              % sample rate (S/s)
fsr         = 2.0;                              % full-scale input range
ampl_dbfs   = -1.0;                             % input amplitude (dBFS)
freq        = 10e6;                             % input frequency (Hz)
phase       = 0.110;                            % input phase (rad)
td          = 0.0;                              % input time delay (s)
tj          = 0.0;                              % RMS aperture uncertainty (s)
poco        = [0.0 1.0 0.0 0.003];              % distortion polynomial coefficients
qres        = 12;                               % quantizer resolution
qnoise_dbfs = -63.0;                            % quantizer noise
code_fmt    = gn.CodeFormatTwosComplement;
sig_type    = gn.DnlSignalTone;
inl_fit     = gn.InlLineFitBestFit;
rfft_scale  = gn.RfftScaleDbfsSin;
window      = gn.WindowNoWindow;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

npts = navg * nfft;
ampl = (fsr / 2) * 10^(ampl_dbfs / 20);
qnoise = 10^(qnoise_dbfs / 20);
ssb_fund = 4;
ssb_rest = 3;
if gn.WindowNoWindow == window
    freq = gn.coherent(nfft, fs, freq);
    ssb_fund = 0;
    ssb_rest = 0;
end

%
% Signal generation and processing
%
awf = gn.cos(npts, fs, ampl, freq, phase, td, tj);
awf = gn.polyval(awf, poco);
qwf = gn.quantize16(awf, fsr, qres, qnoise, code_fmt);
hist = gn.hist(qwf, qres, code_fmt);
dnl = gn.dnl(hist, sig_type);
inl = gn.inl(dnl, inl_fit);
fft_cplx = gn.rfft(qwf, qres, navg, nfft, window, code_fmt, rfft_scale);

%
% Fourier analysis configuration
%
key = 'fa';
gn.mgr_remove(key)
gn.fa_create(key)
gn.fa_analysis_band(key, 'fdata*0.0', 'fdata*1.0')
gn.fa_fixed_tone(key, 'A', gn.FaCompTagSignal, freq, ssb_fund)
% gn.fa_max_tone(key, 'A', gn.FaCompTagSignal, ssb_fund)
gn.fa_hd(key, 3)
gn.fa_ssb(key, gn.FaSsbDefault, ssb_rest)
gn.fa_ssb(key, gn.FaSsbDC, -1)
gn.fa_ssb(key, gn.FaSsbSignal, -1)
gn.fa_ssb(key, gn.FaSsbWO, -1)
gn.fa_fsample(key, fs)
disp(gn.fa_preview(key, false))
% fn = gn.mgr_save(key);

%
% Analysis
%
wf_results   = gn.wf_analysis(qwf);
hist_results = gn.hist_analysis(hist);
dnl_results  = gn.dnl_analysis(dnl);
inl_results  = gn.inl_analysis(inl);
fft_results  = gn.fft_analysis(key, fft_cplx, nfft);

%
% Print results
%
fprintf('\nWaveform Analysis Results:\n')
for k = keys(wf_results)
    fprintf('%-10s%16.6f\n', k{1}, wf_results(k{1}));
end
fprintf('\nHistogram Analysis Results:\n')
for k = keys(hist_results)
    fprintf('%-10s%16.6f\n', k{1}, hist_results(k{1}));
end
fprintf('\nDNL Analysis Results:\n')
for k = keys(dnl_results)
    fprintf('%-10s%16.6f\n', k{1}, dnl_results(k{1}));
end
fprintf('\nINL Analysis Results:\n')
for k = keys(inl_results)
    fprintf('%-10s%16.6f\n', k{1}, inl_results(k{1}));
end
fprintf('\nFourier Analysis Results:\n')
for k = {'fsnr', 'sfdr', 'dc:mag_dbfs', 'A:freq', 'A:ffinal', 'A:mag_dbfs', 'A:phase', '3A:mag_dbc'}
    fprintf('%-20s%20.6f\n', k{1}, fft_results(k{1}))
end

%
% Plot
%
if doplot
    code_axis = gn.code_axis(qres, code_fmt);
    freq_axis = gn.freq_axis(nfft, gn.FreqAxisTypeReal, fs);
    fft_db = gn.db(fft_cplx);
    clf
    subplot(3, 2, 1)
    plot(qwf)
    title('Quantized Waveform')
    subplot(3, 2, 3)
    plot(code_axis, hist)
    title('Histogram')
    subplot(3, 2, 2)
    plot(code_axis, dnl)
    title('DNL')
    subplot(3, 2, 4)
    plot(code_axis, inl)
    title('INL')
    subplot(3, 1, 3)
    plot(freq_axis, fft_db)
    title('FFT')
    xlim([freq_axis(1), freq_axis(end)])
    ylim([-140.0, 20.0])
    grid on
    annots = gn.fa_annotations(fft_results);
    labels = annots('labels');
    for k = 1 : numel(labels)
        xyl = labels{k};
        text(xyl{1}, xyl{2}, xyl{3}, ...
             'HorizontalAlignment', 'center', ...
             'VerticalAlignment', 'bottom')
    end
    lines = annots('lines');
    for k = 1 : numel(lines)
        xyvals = lines{k};
        line(xyvals{1}, xyvals{2}, 'Color', 'm')
    end
    hold on
    ab_boxes = annots('ab_boxes');
    for k = 1 : numel(ab_boxes)
        xyvals = ab_boxes{k};
        fill(xyvals{1}, xyvals{2}, 'k', ...
             'FaceAlpha', 0.1, 'EdgeColor', 'k', 'EdgeAlpha', 0.2)
    end
    tone_boxes = annots('tone_boxes');
    for k = 1 : numel(tone_boxes)
        xyvals = tone_boxes{k};
        fill(xyvals{1}, xyvals{2}, 'm', ...
             'FaceAlpha', 0.1, 'EdgeColor', 'm', 'EdgeAlpha', 0.2)
    end
    hold off
end

genalyzer.unload()