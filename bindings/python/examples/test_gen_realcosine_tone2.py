import sys
sys.path.insert(1, '../')
import genalyzer

c = genalyzer.config_tone_meas(0, 1, 2048, 2, 450000000, 4.5, 3, [12000000, 250000, 7750000], [0, 0.4, 0.5], [-2.0, -3.0, -6.0])
awf = genalyzer.gen_tone(c)
for i in range(4096):
    print(awf[i])

