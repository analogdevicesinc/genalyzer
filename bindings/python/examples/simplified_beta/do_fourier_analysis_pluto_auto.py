# This example shows how to use the Fourier analysis functions with data
# from a PlutoSDR. The data is read from a JSON file. The Fourier analysis
# is configured to automatically find the tones in the data.

import genalyzer, os, json, glob, pprint
import matplotlib.pyplot as plt
import numpy as np

test_dir = os.path.join(*["..", "..", "..", "..", "tests", "test_vectors"])
loc = os.path.dirname(__file__)

f = glob.glob(os.path.join(loc, test_dir, "test_Pluto_DDS_data_1658159639196.json"))
a = open(f[0])
data = json.load(a)
if data["num_tones"] == 1:
    freq_list = [data["freq"]]
else:
    freq_list = data["freq"]

qwfi = data["test_vec_i"]
qwfi = [int(i) for i in qwfi]
qwfq = data["test_vec_q"]
qwfq = [int(i) for i in qwfq]

# configure
c = genalyzer.simplified_beta.config_fftz(
    data["npts"], data["qres"], data["navg"], data["nfft"], data["win"] - 1
)
genalyzer.simplified_beta.config_set_sample_rate(data["fs"], c)

# Find tones
genalyzer.simplified_beta.config_fa_auto(ssb_width=120, c=c)

# compute FFT
fft_out_i, fft_out_q = genalyzer.simplified_beta.fftz(qwfi, qwfq, c)
fft_out = [val for pair in zip(fft_out_i, fft_out_q) for val in pair]

# get all Fourier analysis results
all_results = genalyzer.simplified_beta.get_fa_results(fft_out, c)

# plot FFT
fft_cp = np.empty(data["nfft"], dtype=np.complex128)
fft_cp.real = [fft_out[i] for i in range(len(fft_out)) if i % 2 != 0]
fft_cp.imag = [fft_out[i] for i in range(len(fft_out)) if i % 2 != 1]
fft_cp = np.roll(np.absolute(fft_cp), round(0.5 * data["nfft"]))
f = np.linspace(start=-0.5 * data["fs"], stop=0.5 * data["fs"], num=data["nfft"])
dbfs_data = 10 * np.log10(np.square(fft_cp))
plt.clf()
plt.plot(f, dbfs_data)

# Add markers for the harmonics
harmonic_keys = ["A", "2A", "dc"]
for key in harmonic_keys:
    freq = all_results[f"{key}:freq"]
    amp = all_results[f"{key}:mag_dbfs"]
    plt.plot(freq, amp, "ro")


plt.ylim([-140, 0])
plt.xlabel("frequency [Hz]")
plt.ylabel("PSD [dBFs]")
plt.draw()
plt.show()

# get a single Fourier analysis result
sfdr = genalyzer.simplified_beta.get_fa_single_result("sfdr", fft_out, c)

# display results
pprint.pprint(all_results)
print("SFDR - ", sfdr)

# free memory
genalyzer.simplified_beta.config_free(c)
