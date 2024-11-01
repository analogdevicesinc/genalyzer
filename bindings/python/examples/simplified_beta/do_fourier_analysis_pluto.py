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
genalyzer.simplified_beta.config_fa(freq_list[0], c)

# compute FFT
fft_out_i, fft_out_q = genalyzer.simplified_beta.fftz(qwfi, qwfq, c)
fft_out = [val for pair in zip(fft_out_i, fft_out_q) for val in pair]

# plot FFT
fft_cp = np.empty(data["nfft"], dtype=np.complex128)
fft_cp.real = [fft_out[i] for i in range(len(fft_out)) if i % 2 != 0]
fft_cp.imag = [fft_out[i] for i in range(len(fft_out)) if i % 2 != 1]
fft_cp = np.roll(np.absolute(fft_cp), round(0.5 * data["nfft"]))
f = np.linspace(start=-0.5 * data["fs"], stop=0.5 * data["fs"], num=data["nfft"])
plt.clf()
plt.plot(f, 10 * np.log10(np.square(fft_cp)))
plt.ylim([-140, 0])
plt.xlabel("frequency [Hz]")
plt.ylabel("PSD [dBFs]")
plt.draw()
plt.show()

# get a single Fourier analysis result
sfdr = genalyzer.simplified_beta.get_fa_single_result("sfdr", fft_out, c)

# display results
print("SFDR - ", sfdr)

# free memory
genalyzer.simplified_beta.config_free(c)
