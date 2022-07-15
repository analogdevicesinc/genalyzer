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
awf1 = genalyzer.gen_real_tone(c)
genalyzer.config_quantize(npts, fsr, qres, qnoise, c)
qwf1 = genalyzer.quantize(awf1, c)
plt.plot(awf1[:1000])
plt.plot(qwf1[:1000])

c = genalyzer.config_gen_tone(ttype, npts, sample_rate, num_tones, tone_freq, tone_ampl, tone_phase)
awf2_i, awf2_q = genalyzer.gen_complex_tone(c)
genalyzer.config_quantize(npts, fsr, qres, qnoise, c)
qwf2_i = genalyzer.quantize(awf2_i, c)
qwf2_q = genalyzer.quantize(awf2_q, c)
plt.plot(awf2_i[:1000])
plt.plot(awf2_q[:1000])
plt.plot(qwf2_i[:1000])
plt.plot(qwf2_q[:1000])

print(awf2_i[:10])
print(awf2_q[:10])
print(qwf2_i[:10])
print(qwf2_q[:10])

genalyzer.config_free(c)