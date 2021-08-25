from ctypes import *
libgen = CDLL('../../build/wrapper_src/libgenalyzer.so')

class _Opaque(Structure):
    pass

gen_tone = libgen.gen_tone
gen_tone.restype = None
gen_tone.argtypes = [POINTER(_Opaque), POINTER(POINTER(c_double))]

c = POINTER(_Opaque)()
domain_wf = c_uint(0)
type_wf = c_uint(1)
nfft = c_ulong(2048)
navg = c_int(2)
fs = c_double(450000000)
fsr = c_double(4.5)
res = c_int(0)
num_tones = c_ulong(3)
double_array = c_double*num_tones.value
freq_arr = [12000000, 250000, 7750000]
freq = double_array(*freq_arr)
phase = double_array(0, 0.4, 0.5)
scale = double_array(-2, -3, -6)
fsample_update = c_bool(False)
fdata_update = c_bool(False)
fshift_update = c_bool(False)

config_tone_meas = libgen.config_tone_meas
config_tone_meas.restype = None
config_tone_meas.argtypes = [POINTER(POINTER(_Opaque)), c_uint, c_uint, c_ulong, c_int, c_double, c_double, c_int, c_double*num_tones.value, POINTER(c_double*num_tones.value), POINTER(c_double*num_tones.value), c_ulong, c_bool, c_bool, c_bool]

config_tone_meas(byref(c), domain_wf, type_wf, nfft, navg, fs, fsr, res, freq, scale, phase, num_tones, fsample_update, fdata_update, fshift_update)

awf = POINTER(c_double)()
gen_tone(c, byref(awf))
