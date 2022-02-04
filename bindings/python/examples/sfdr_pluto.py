import time 
import adi
import matplotlib.pyplot as plt
import numpy as np
from scipy import signal
import genalyzer


# Create radio
sdr = adi.Pluto()

# Configure properties
sdr.rx_lo = 2400000000
sdr.tx_lo = 2400000000
sdr.tx_cyclic_buffer = True
sdr.tx_hardwaregain_chan0 = -10
sdr.gain_control_mode_chan0 = "fast_attack"
sdr.filter = "LTE10_MHz.ftr"

fs = int(sdr.sample_rate)
tone_freq_hz = 2000000  # In Hz
tone_scale = 1.0  # Range: 0-1.0
tx_channel = 0  # Starts at 0
sdr.dds_single_tone(tone_freq_hz, tone_scale, tx_channel)

# Collect data
for r in range(20):
    x = sdr.rx()

x_re = x.real
x_im = x.imag
x_intrlv = np.empty(2*x.size, dtype=x_re.dtype)
x_intrlv[0::2] = x_re
x_intrlv[1::2] = x_im

# setup measurement
inputs = dict()
inputs["domain_wf"] = 0
inputs["type_wf"] = 2
inputs["nfft"] = len(x)
inputs["navg"] = 1
inputs["fs"] = fs
inputs["fsr"] = 0
inputs["res"] = 12
inputs["win"] = 1
inputs["freq"] = [0]
inputs["phase"] = [0]
inputs["scale"] = [0]
config_obj = genalyzer.gn_params(**inputs)
c = genalyzer.config_tone_meas(config_obj)

# compute metric
result, fft_i, fft_q, err_code = genalyzer.metric_t(c, x_intrlv, "SFDR")
print('SFDR: ', result)

# plot FFT
fft_x = np.empty(len(fft_i), dtype=np.complex)
fft_x.real = fft_i
fft_x.imag = fft_q
fft_x = np.roll(np.absolute(fft_x), round(0.5*len(x)))
f = np.linspace(start=-0.5*fs, stop=0.5*fs, num=len(x))
plt.clf()
plt.plot(f, 10*np.log10(np.square(fft_x)))
plt.ylim([-140, 0])
plt.xlabel("frequency [Hz]")
plt.ylabel("PSD [dBFs]")
plt.draw()
plt.show()