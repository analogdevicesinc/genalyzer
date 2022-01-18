clc;
clear all;
close all;

%% Tx set up
tx = adi.Pluto.Tx;
tx.uri = 'ip:pluto'; 
tx.DataSource = 'DDS';
tx.DDSFrequencies = [2e6 2e6; 0 0]; % set DDS complex tone freq to 2 MHz 
tx.DDSPhases = [90e3 0; 0 0]; % expressed in millidegrees
tx.DDSScales = [1 1; 0 0];
tx.CenterFrequency = 2.4e9;
tx.EnableCustomFilter = true;
tx.CustomFilterFileName = 'LTE10_MHz.ftr'; % sets PlutoSDR sample-rate to 15.36 MSPS
tx.AttenuationChannel0 = -10;
tx();
pause(1);

%% Rx set up
rx = adi.Pluto.Rx('uri','ip:pluto');
rx.CenterFrequency = tx.CenterFrequency;
rx.EnableCustomFilter = true;
rx.CustomFilterFileName = 'LTE10_MHz.ftr';
rx.GainControlModeChannel0 = 'fast_attack';
y = rx();
y_re = real(y);
y_im = imag(y);

tx.release();
rx.release();

figure(1); 
plot(0:numel(y)-1, real(y), 'r', 0:numel(y)-1, imag(y), 'b'); 
xlim([0 250]); 
xlabel('sample index'); 
grid on;

% load library
load_genalyzer();

% measurement settings 
domain = 0; % 0 - TIME; 1 - FREQ
type = 2; 
nfft = length(y);
navg = 1;
fs = 15.36e6;
fsr = 0;
res = 12;
window = 1; % 0 - BlackmanHarris, 1 - Hann, 2 - Rect

% setup measurement for time-domain measurement
c = libpointer; % create a null pointer
calllib('libgenalyzer', 'gn_config_tone_meas', ...
    c, domain, type, nfft, navg, fs, fsr, res, window, false, false, false);

% calculating SFDR using the time-domain waveform
err_code = libpointer('uint32Ptr',0);
metric = char('SFDR');
fft_time_re = libpointer('doublePtrPtr', zeros(nfft, 1));
fft_time_im = libpointer('doublePtrPtr',  zeros(nfft, 1));
y_interleaved = [y_re'; y_im'];
y_interleaved = y_interleaved(:);
y_interleavedPtr = libpointer('int32Ptr', y_interleaved);
sfdr_time = calllib('libgenalyzer', 'gn_metric', c, y_interleavedPtr, metric, fft_time_re, fft_time_im, err_code);
fprintf('SFDR (time) - %f\terror code - %d\n', sfdr_time, err_code.Value);

% plot FFT computed by genalyzer
fft_gen_time = fft_time_re.Value+1i*fft_time_im.Value;
f = (-nfft/2:nfft/2-1)*fs/nfft;
psd_y = circshift(20*log10(abs(fft_gen_time)), nfft/2);
figure; plot(f*1e-6, psd_y);
grid on;
axis square;
xlabel('freq (MHz)');
ylabel('magnitude (dBFs)');
xlim([-8 8]);

% fft can be calculated independently as well
fft_re = libpointer('doublePtrPtr', zeros(nfft, 1));
fft_im = libpointer('doublePtrPtr',  zeros(nfft, 1));
fft_len = libpointer('uint64Ptr',0);
calllib('libgenalyzer', 'gn_fft', c, y_re, y_im, fft_re, fft_im, fft_len);

% setup measurement for freq-domain measurement
c = libpointer;
domain = 1; % 0 - TIME; 1 - FREQ
calllib('libgenalyzer', 'gn_config_tone_meas', ...
    c, domain, type, nfft, navg, fs, fsr, res, window, false, false, false);

% calculating SFDR using the FFT calculated from time-domain waveform
err_code = libpointer('uint32Ptr',0);
fft_y = fft(hann(length(y)).*y);
ffty_interleaved = [real(fft_y)'; imag(fft_y).'];
ffty_interleaved = ffty_interleaved(:);
ffty_interleavedPtr = libpointer('doublePtr', ffty_interleaved);
fft_freq_re = libpointer('doublePtrPtr', zeros(nfft, 1));
fft_freq_im = libpointer('doublePtrPtr',  zeros(nfft, 1));
sfdr_freq = calllib('libgenalyzer', 'gn_metric', c, ffty_interleavedPtr, metric, fft_freq_re, fft_freq_im, err_code);
fprintf('SFDR (freq) - %f\terror code - %d\n', sfdr_freq, err_code.Value);

% unload library
clear c metric err_code fft_re fft_im fft_len;
unloadlibrary('libgenalyzer');