import genalyzer
import matplotlib.pyplot as plt


ttype = 0
npts = 16384
sample_rate = 900000000.0
num_tones = 3
tone_freq = [250000.0, 4750000.0, 14750000.0]
tone_ampl = [0.25, 1, 1]
tone_phase = [0.3, 0.3, 0.2]
fsr = 3.0
qres = 12
qnoise = 0.0

c = genalyzer.config_gen_tone(ttype, npts, sample_rate, num_tones, tone_freq, tone_ampl, tone_phase)
awf = genalyzer.gen_real_tone(c)
genalyzer.config_quantize(npts, fsr, qres, qnoise, c)
qwf = genalyzer.quantize(awf, c)
plt.plot(awf[:1000])
plt.plot(qwf[:1000])
genalyzer.config_free(c)