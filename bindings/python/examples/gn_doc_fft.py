import numpy as np
import genalyzer.advanced as gn
import matplotlib.pyplot as pl

#
# Signal 
#
npts = 30000 # number of points in the signal
fs = 3e6 # sample-rate of the data
freq = 300000 # tone frequency
phase = 0.0 # tone phase
ampl_dbfs = -1.0 # amplitude of the tone in dBFS
qnoise_dbfs = -60.0  # quantizer noise in dBFS
fsr = 2.0 # full-scale range of I/Q components of the complex tone
ampl = (fsr / 2) * 10 ** (ampl_dbfs / 20) # amplitude of the tone in linear scale
qnoise = 10 ** (qnoise_dbfs / 20) # quantizer noise in linear scale
qres = 12  # data resolution
code_fmt = gn.CodeFormat.TWOS_COMPLEMENT # integer data format

#
# Genarate signal for analysis
#
awfi = gn.cos(npts, fs, ampl, freq, phase)
awfq = gn.sin(npts, fs, ampl, freq, phase)
qwfi = gn.quantize(awfi, fsr, qres, qnoise, code_fmt)
qwfq = gn.quantize(awfq, fsr, qres, qnoise, code_fmt)

#
# FFT configuration
#
navg = 1 # number of FFT averages
nfft = int(npts/navg) # FFT-order
window = gn.Window.NO_WINDOW # window function to apply
axis_type = gn.FreqAxisType.DC_CENTER # axis type
axis_fmt = gn.FreqAxisFormat.FREQ # axis-format

#
# Compute FFT
#
fft_cplx = gn.fft(qwfi, qwfq, qres, navg, nfft, window, code_fmt)
freq_axis = gn.freq_axis(nfft, axis_type, fs, axis_fmt)
fft_db = gn.db(fft_cplx)
if gn.FreqAxisType.DC_CENTER == axis_type:
    fft_db = gn.fftshift(fft_db)

#
# Plot
#
scale_MHz = 1e-6
fig = pl.figure(1)
fig.clf()
pl.plot(freq_axis*scale_MHz, fft_db)
pl.grid(True)
pl.xlabel('frequency (MHz)')
pl.ylabel('magnitude (dBFs)')
pl.xlim(freq_axis[0]*scale_MHz, freq_axis[-1]*scale_MHz)
pl.ylim(-140.0, 20.0)
pl.savefig('fft.png')