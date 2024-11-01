import genalyzer, pprint

c = genalyzer.simplified_beta.config_gen_ramp(8192, 0, 2)
genalyzer.simplified_beta.config_quantize(8192, 3.0, 12, pow(10.0, -60.0 / 20.0), c)

awf = genalyzer.simplified_beta.gen_ramp(c)
qwf = genalyzer.simplified_beta.quantize(awf, c)
hist = genalyzer.simplified_beta.histz(qwf, c)
ha_results = genalyzer.simplified_beta.get_ha_results(hist, c)
pprint.pprint(ha_results)
genalyzer.simplified_beta.config_free(c)