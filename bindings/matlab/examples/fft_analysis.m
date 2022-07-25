genalyzer.load()
gn = genalyzer;
fprintf('Data Converter Analysis Library\nVersion: %s\n', gn.version())

%
% Setup
%
doplot      = true;
navg        = 2;                                % FFT averaging number
nfft        = 1024 * 16;                        % FFT size
fs          = 1e9;                              % sample rate (S/s)
fdata       = fs / 1;                           % data rate, e.g., the downsampled rate (S/s)
fshift      = 0e6;                              % shift frequency, e.g., NCO frequency (Hz)
fsr         = 2.0;                              % full-scale input range
ampl_dbfs   = -1.0;                             % input amplitude (dBFS)
freq        = 70e6;                             % input frequency (Hz)
phase       = 0.110;                            % input phase (rad)
td          = 0.0;                              % input time delay (s)
tj          = 0.0;                              % RMS aperture uncertainty (s)
qpe         = (pi / 2) * 1e-5;                  % quadrature phase error (rad)
poco        = [0.0 1.0 0.0 0.003];              % distortion polynomial coefficients
qres        = 12;                               % quantizer resolution
qnoise_dbfs = -63.0;                            % quantizer noise (dBFS)
code_fmt    = gn.CodeFormatTwosComplement;
axis_fmt    = gn.FreqAxisFormatFreq;
axis_type   = gn.FreqAxisTypeDcCenter;
window      = gn.WindowNoWindow;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

dsr = fs / fdata;                               % downsample ratio
npts = navg * nfft * dsr;                       % number of simulation points
ampl = (fsr / 2) * 10^(ampl_dbfs / 20);
qnoise = 10^(qnoise_dbfs / 20);
ssb_fund = 4;
ssb_rest = 3;
if gn.WindowNoWindow == window
    freq = gn.coherent(nfft, fdata, freq);
    fbin = fdata / nfft;
    fshift = round(fshift / fbin) * fbin;
    ssb_fund = 0;
    ssb_rest = 0;
end

%
% Signal generation and processing
%
awfi = gn.cos(npts, fs, ampl, freq, phase, td, tj);
awfq = gn.sin(npts, fs, ampl, freq, phase + qpe, td, tj);
awfi = gn.polyval(awfi, poco);
awfq = gn.polyval(awfq, poco);
qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt);
qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt);
xwf = gn.fshift(qwfi, qwfq, qres, fs, fshift, code_fmt);
xwf = gn.downsample(xwf, dsr, true);
fft_cplx = gn.fft(xwf, qres, navg, nfft, window, code_fmt);

%
% Fourier analysis configuration
%
key = 'fa';
gn.mgr_remove(key)
gn.fa_create(key)
gn.fa_analysis_band(key, 'fdata*0.0', 'fdata*1.0')
gn.fa_fixed_tone(key, 'A', gn.FaCompTagSignal, freq)
% gn.fa_max_tone(key, 'A', gn.FaCompTagSignal, ssb_fund)
gn.fa_conv_offset(key, 0.0 ~= fshift)
gn.fa_hd(key, 3)
gn.fa_ssb(key, gn.FaSsbDefault, ssb_rest)
gn.fa_ssb(key, gn.FaSsbDC, -1)
gn.fa_ssb(key, gn.FaSsbSignal, -1)
gn.fa_ssb(key, gn.FaSsbWO, -1)
gn.fa_fdata(key, fdata)
gn.fa_fsample(key, fs)
gn.fa_fshift(key, fshift)
disp(gn.fa_preview(key, true))
% fn = gn.mgr_save(key);

%
% Fourier analysis execution
%
results = gn.fft_analysis(key, fft_cplx, nfft, axis_type);
carrier = gn.fa_result_string(results, 'carrierindex');
maxspur = gn.fa_result_string(results, 'maxspurindex');

%
% Print results
%
for k = {'fsnr', 'sfdr', 'dc:mag_dbfs', 'A:freq', 'A:ffinal', 'A:mag_dbfs', 'A:phase', '-3A:mag_dbc'}
    fprintf('%-20s%20.6f\n', k{1}, results(k{1}))
end
fprintf('%-20s%20s\n', 'Carrier', carrier)
fprintf('%-20s%20s\n', 'MaxSpur', maxspur)

if doplot
    freq_axis = gn.freq_axis(nfft, axis_type, fdata, axis_fmt);
    fft_db = gn.db(fft_cplx);
    if gn.FreqAxisTypeDcCenter == axis_type
        fft_db = gn.fftshift(fft_db);
    end
    clf
    plot(freq_axis, fft_db)
    xlim([freq_axis(1), freq_axis(end)])
    ylim([-140.0, 20.0])
    grid on
    annots = gn.fa_annotations(results, axis_type, axis_fmt);
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