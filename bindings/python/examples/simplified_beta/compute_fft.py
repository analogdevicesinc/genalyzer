import genalyzer, os, json, glob
import matplotlib.pyplot as plt

test_dir = os.path.join(*["..", "..", "..", "..", "tests", "test_vectors"])
loc = os.path.dirname(__file__)

f = glob.glob(os.path.join(loc, test_dir, "test_fft_tone_1655478044632.json"))
a = open(f[0])
data = json.load(a)
if data["num_tones"] == 1:
    freq_list = [data["freq"]]
else:
    freq_list = data["freq"]

qwfi = data["test_vecq_i"]
qwfi = [int(i) for i in qwfi]
qwfq = data["test_vecq_q"]
qwfq = [int(i) for i in qwfq]
c = genalyzer.simplified_beta.config_fftz(
    data["npts"], data["qres"], data["navg"], data["nfft"], data["win"] - 1
)
fft_out_i, fft_out_q = genalyzer.simplified_beta.fftz(qwfi, qwfq, c)
plt.plot(fft_out_i[:1000])
plt.plot(fft_out_q[:1000])
genalyzer.simplified_beta.config_free(c)
