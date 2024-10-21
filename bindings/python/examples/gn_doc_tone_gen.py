import numpy as np
import genalyzer.advanced as gn
import matplotlib.pyplot as pl
import pprint

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

# Plot
time_axis = np.arange(0, npts*1/fs, 1/fs)
fig = pl.figure(1)
fig.clf()
pl.plot(time_axis*1e6, awfi, 'r+-', label="cosine")
pl.plot(time_axis*1e6, awfq, 'b.-', label="sine")
pl.grid(True)
pl.xlabel('time (us)')
pl.ylabel('magnitude')
pl.xlim(time_axis[0], time_axis[50]*1e6)
pl.ylim(-1.2, 1.2)
pl.legend(loc="upper right")
pl.savefig('complex_sinusoidal_waveform.png')

wfa_results_i = gn.wf_analysis(qwfi)
wfa_results_q = gn.wf_analysis(qwfq)
print('+---------------------------+')
print("waveform-analysis results (I)")
print('+---------------------------+')
pprint.pprint(wfa_results_i)
print('+---------------------------+')
print("waveform-analysis results (Q)")
print('+---------------------------+')
pprint.pprint(wfa_results_q)