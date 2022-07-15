import genalyzer, os, json, glob, pprint

test_dir = os.path.join(*["..", "..", "..", "tests", "test_vectors"])
loc = os.path.dirname(__file__)

f = glob.glob(os.path.join(loc, test_dir, "test_quantize_real_tone_1655388332014.json"))
a = open(f[0])
data = json.load(a)

qwf = data['test_vecq']
qwf = [int(i) for i in qwf]
c = genalyzer.config_quantize(data['npts'], data['fsr'], data['qres'], data['qnoise'])
wfa_results = genalyzer.get_wfa_results(qwf, c)
pprint.pprint(wfa_results)
genalyzer.config_free(c)