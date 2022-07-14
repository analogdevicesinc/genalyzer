import genalyzer, os, json, glob, pprint
import matplotlib.pyplot as plt

test_dir = os.path.join(*["..", "..", "..", "tests", "test_vectors"])
loc = os.path.dirname(__file__)

f = glob.glob(os.path.join(loc, test_dir, "test_fft_tone_1655478044632.json"))
a = open(f[0])
data = json.load(a)
if data['num_tones'] == 1:
    freq_list = [data['freq']]
else:
    freq_list = data['freq']

qwfi = data['test_vecq_i']
qwfi = [int(i) for i in qwfi]
qwfq = data['test_vecq_q']
qwfq = [int(i) for i in qwfq]
c = genalyzer.config_fftz(data['npts'], data['qres'], data['navg'], data['nfft'], data['win']-1)
fft_out_i, fft_out_q = genalyzer.fftz(qwfi, qwfq, c)
fft_out = [val for pair in zip(fft_out_i, fft_out_q) for val in pair]
genalyzer.config_fa(freq_list[0], c)
all_results = genalyzer.get_fa_results(fft_out, c)
pprint.pprint(all_results)
sfdr = genalyzer.get_fa_single_result("sfdr", fft_out, c)
genalyzer.config_free(c)
print(sfdr)