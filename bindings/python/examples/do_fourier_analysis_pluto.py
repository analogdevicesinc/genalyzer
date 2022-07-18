import genalyzer, os, json, glob, pprint
import matplotlib.pyplot as plt

test_dir = os.path.join(*["..", "..", "..", "tests", "test_vectors"])
loc = os.path.dirname(__file__)

f = glob.glob(os.path.join(loc, test_dir, "test_Pluto_DDS_data_1658159639196.json"))
a = open(f[0])
data = json.load(a)
if data['num_tones'] == 1:
    freq_list = [data['freq']]
else:
    freq_list = data['freq']

qwfi = data['test_vec_i']
qwfi = [int(i) for i in qwfi]
qwfq = data['test_vec_q']
qwfq = [int(i) for i in qwfq]

# configure
c = genalyzer.config_fftz(data['npts'], data['qres'], data['navg'], data['nfft'], data['win']-1)
genalyzer.config_fa(freq_list[0], c)

# compute FFT
fft_out_i, fft_out_q = genalyzer.fftz(qwfi, qwfq, c)
fft_out = [val for pair in zip(fft_out_i, fft_out_q) for val in pair]

# get all Fourier analysis results
all_results = genalyzer.get_fa_results(fft_out, c)

# get a single Fourier analysis result
sfdr = genalyzer.get_fa_single_result("sfdr", fft_out, c)

# display results
pprint.pprint(all_results)
print('SFDR - ', sfdr)

# free memory
genalyzer.config_free(c)