<h1>Theory</h1>

Spurious free dynamic range (SFDR) is an important performance specification of ADCs. It specifies the capability of the ADC and the system to decipher a carrier signal from other noise or any other spurious frequency. It represents the smallest power signal that can be distinguished from a large interfering signal. 

Mathematically, it is the ratio between the root mean square (rms) value of the power of a carrier and the rms value of the next most significant spurious signal seen in the frequency domain, such as in a fast Fourier transform (FFT). Hence, by definition, this dynamic range must be free of other spurious frequencies, or spurs. 

SFDR is quantified as the range, in units of power (dBc), relative from the carrier of interest to the power of the next most significant frequency. However, it also could be referenced to a full-scale signal in units of power (dBFS). This is an important distinction since the carrier of interest may be a relatively lower power signal that is well below the full-scale input to the ADC. When this is the case, the SFDR becomes paramount in distinguishing the signal from other noise and spurious frequencies.

A harmonic frequency is an integer multiple of the fundamental frequency. Hence, the SFDR typically will be dominated by the dynamic range between a carrier frequency and the second or third harmonic of the fundamental frequency of interest. 

<h1>Demonstration Using genalyzer and ADALM-PLUTO</h1>

In this example, these ideas are demonstrated by using MATLAB's `calllib` interface to genalyzer. A link to the complete MATLAB script is shown at the bottom of this page. 

An ADALM-PLUTO is connected in RF loopback and is configured to transmit a 2 MHz complex tone, with the sample rate set to 15.36 MSPS. The corresponding code block is shown below. For more details, see (link to TRx Toolbox).
``` py
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
```

Before MATLAB bindings can be used, the libraray needs to be loaded into MATLAB workspace, which is done by calling `load_genalyzer()` function. 

As explained in section xyz, the first step is to create an opaque configuration `struct`. Using MATLAB bindings, this is done as shown by the code snippet below. 
``` py
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
``` 

Now, the configuration `struct` contains the measurement settings in order to calculate the desired metric. SFDR calculation is done as shown in the following MATLAB code snippet. 
``` py
% calculating SFDR using the time-domain waveform
err_code = libpointer('uint32Ptr',0);
fft_len = libpointer('uint64Ptr',0);
metric = char('SFDR');
fft_time_re = libpointer('doublePtrPtr', zeros(nfft, 1));
fft_time_im = libpointer('doublePtrPtr',  zeros(nfft, 1));
y_interleaved = [y_re'; y_im'];
y_interleaved = y_interleaved(:);
y_interleavedPtr = libpointer('int32Ptr', y_interleaved);
sfdr_time = calllib('libgenalyzer', 'gn_metric', c, y_interleavedPtr, metric, fft_time_re, fft_time_im, fft_len, err_code);
``` 

Note that the corresponding FFT calculated is returned by genalyzer. Plotting the FFT computed using genalyzer, we can see that the second harmonic at 4 MHz and its image dominate the remaining spurs. Note that the third harmonic and its image are filtered out by the programmable filter coefficients loaded into ADALM-PLUTO. 
<figure markdown>
  ![ADALM-PLUTO FFT](assets/PlutoSDR_FFT.svg){ width="900" }
</figure> 

Since SFDR is by definition, free of spurious frequencies, by examining the above plot, one can estimate SFDR to be around 40 dBc. This values agrees with the SFDR computed by genalyzer. 
``` py
SFDR (time) - 42.885192	error code - 0
```

Note that genalyzer can be configured to compute performance metrics from frequency-domain data as well. This is shown by the code snippet below. 
``` py
err_code = libpointer('uint32Ptr',0);
fft_len = libpointer('uint64Ptr',0);
fft_y = fft(hann(length(y)).*y);
ffty_interleaved = [real(fft_y)'; imag(fft_y).'];
ffty_interleaved = ffty_interleaved(:);
ffty_interleavedPtr = libpointer('doublePtr', ffty_interleaved);
fft_freq_re = libpointer('doublePtrPtr', zeros(nfft, 1));
fft_freq_im = libpointer('doublePtrPtr',  zeros(nfft, 1));
sfdr_freq = calllib('libgenalyzer', 'gn_metric', c, ffty_interleavedPtr, metric, fft_freq_re, fft_freq_im, fft_len, err_code);
```
The result obtained matches the corresponding result obtained using time-domain waveform closely.
```
SFDR (freq) - 42.885226	error code - 0
```