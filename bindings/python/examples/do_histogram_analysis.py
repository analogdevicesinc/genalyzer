import genalyzer, pprint

c = genalyzer.config_gen_ramp(8192, 0, 2)
genalyzer.config_quantize(8192, 3.0, 12, pow(10.0, -60.0 / 20.0), c)

awf = genalyzer.gen_ramp(c)
qwf = genalyzer.quantize(awf, c)
hist = genalyzer.histz(qwf, c)
ha_results = genalyzer.get_ha_results(hist, c)
pprint.pprint(ha_results)
genalyzer.config_free(c)