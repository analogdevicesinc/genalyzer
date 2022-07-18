import genalyzer, pprint

c = genalyzer.config_gen_tone(0, 8192, 5000000.0, 1, [50000.0], [0.5], [0.2])
genalyzer.config_quantize(8192, 3.0, 12, pow(10.0, -60.0 / 20.0), c)

awf = genalyzer.gen_real_tone(c)
qwf = genalyzer.quantize(awf, c)
wfa_results = genalyzer.get_wfa_results(qwf, c)
pprint.pprint(wfa_results)
genalyzer.config_free(c)