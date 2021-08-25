from typing import List
from ctypes import *

_libgen = CDLL("libgenalyzer.so")

class _Config(Structure):
    pass

_config_tone_meas = _libgen.config_tone_meas
_config_tone_meas.restype = None
_config_tone_meas.argtypes = [
    POINTER(POINTER(_Config)),
    c_uint,
    c_uint,
    c_ulong,
    c_int,
    c_double,
    c_double,
    c_int,
    POINTER(c_double),
    POINTER(c_double),
    POINTER(c_double),
    c_ulong,
    c_bool,
    c_bool,
    c_bool,
]

_config_noise_meas = _libgen.config_noise_meas
_config_noise_meas.restype = None
_config_noise_meas.argtypes = [
    POINTER(POINTER(_Config)),
    c_uint,
    c_ulong,
    c_int,
    c_double,
    c_double,
    c_int,
    c_double,
    c_bool,
    c_bool,
    c_bool,
]


_config_tone_nl_meas = _libgen.config_tone_nl_meas
_config_tone_nl_meas.restype = None
_config_tone_nl_meas.argtypes = [
    POINTER(POINTER(_Config)),
    c_uint,
    c_ulong,
    c_double,
    c_double,
    c_int,
    POINTER(c_double),
    POINTER(c_double),
    POINTER(c_double),
    c_ulong
]

_config_ramp_nl_meas = _libgen.config_ramp_nl_meas
_config_ramp_nl_meas.restype = None
_config_ramp_nl_meas.argtypes = [
    POINTER(POINTER(_Config)),
    c_ulong,
    c_double,
    c_double,
    c_int,
    c_double,
    c_double,
    c_double
]

_gen_tone = _libgen.gen_tone
_gen_tone.restype = None
_gen_tone.argtypes = [POINTER(_Config), POINTER(POINTER(c_double)), POINTER(c_uint)]

_gen_noise = _libgen.gen_noise
_gen_noise.restype = None
_gen_noise.argtypes = [POINTER(_Config), POINTER(c_uint), POINTER(POINTER(c_double))]

_gen_ramp = _libgen.gen_ramp
_gen_ramp.restype = None
_gen_ramp.argtypes = [POINTER(_Config), POINTER(POINTER(c_double))]

_quantize = _libgen.quantize
_quantize.restype = None
_quantize.argtypes = [POINTER(_Config), POINTER(c_double), POINTER(POINTER(c_int))]

_rfft = _libgen.rfft
_rfft.restype = None
_rfft.argtypes = [POINTER(_Config), POINTER(c_int), POINTER(POINTER(c_double)), POINTER(POINTER(c_double)), POINTER(c_uint)]

_fft = _libgen.fft
_fft.restype = None
_fft.argtypes = [POINTER(_Config), POINTER(c_int), POINTER(c_int), POINTER(POINTER(c_double)), POINTER(POINTER(c_double)), POINTER(c_uint)]

def config_tone_meas(d: dict) -> POINTER(_Config):
    c = POINTER(_Config)()
    domain_wf = c_uint(int(d['domain_wf']))
    type_wf = c_uint(int(d['type_wf']))
    nfft = c_ulong(int(d['nfft']))
    navg = c_int(int(d['navg']))
    fs = c_double(float(d['fs']))
    fsr = c_double(float(d['fsr']))
    res = c_int(int(d['res'])) if ("res" in d.keys()) else c_int(0)
    freq = [d[key] for key in d.keys() if key.startswith('freq')]
    phase = [d[key] for key in d.keys() if key.startswith('phase')]
    scale = [d[key] for key in d.keys() if key.startswith('scale')]
    assert len(freq) == len(phase), "number of frequency values need to match number of phase values"
    assert len(phase) == len(scale), "number of phase values need to match number of scale values"
    num_tones = c_ulong(len(freq))        
    double_array = c_double * num_tones.value
    freq = (double_array)(*freq)
    phase = (double_array)(*phase)
    scale = (double_array)(*scale)
    fsample_update = c_bool(False)
    fdata_update = c_bool(False)
    fshift_update = c_bool(False)

    _config_tone_meas(
        byref(c),
        domain_wf,
        type_wf,
        nfft,
        navg,
        fs,
        fsr,
        res,
        freq,
        scale,
        phase,
        num_tones,
        fsample_update,
        fdata_update,
        fshift_update,
    )
    return c


def config_noise_meas(
    type_wf: int,
    nfft: int,
    navg: int,
    fs: float,
    fsr: float,
    res: int,
    npr: float,    
    fsample_update: bool = False,
    fdata_update: bool = False,
    fshift_update: bool = False,
) -> POINTER(_Config):
    c = POINTER(_Config)()
    type_wf = c_uint(type_wf)
    nfft = c_ulong(nfft)
    navg = c_int(navg)
    fs = c_double(fs)
    fsr = c_double(fsr)
    res = c_int(0)
    npr = c_double(npr)
    fsample_update = c_bool(fsample_update)
    fdata_update = c_bool(fdata_update)
    fshift_update = c_bool(fshift_update)

    _config_noise_meas(
        byref(c),
        type_wf,
        nfft,
        navg,
        fs,
        fsr,
        res,
        npr,
        fsample_update,
        fdata_update,
        fshift_update,
    )
    return c


def config_ramp_nl_meas(d: dict) -> POINTER(_Config):
    c = POINTER(_Config)()
    npts = c_ulong(int(d['npts']))
    fs = c_double(float(d['fs']))
    fsr = c_double(float(d['fsr']))
    res = c_int(0)
    assert d['start'] < d['stop'], "ramp waveform start value needs to be less than ramp stop value"
    start = c_double(float(d['start']))
    stop = c_double(float(d['stop']))

    _config_ramp_nl_meas(
        byref(c),
        npts,
        fs,
        fsr,
        res,
        start,
        stop,
        0.0
    )
    return c

def config_tone_nl_meas(
    type_wf: int,
    npts: int,
    fs: float,
    fsr: float,
    res: int, 
    num_tones: int,
    freq: List[int],
    phase: List[int],
    scale: List[int],
) -> POINTER(_Config):
    c = POINTER(_Config)()
    domain_wf = c_uint(domain_wf)
    npts = c_ulong(npts)
    fs = c_double(fs)
    fsr = c_double(fsr)
    res = c_int(0)
    num_tones = c_ulong(num_tones)
    double_array = c_double * num_tones.value
    freq = double_array(*freq)
    phase = double_array(*phase)
    scale = double_array(*scale)

    _config_tone_nl_meas(
        byref(c),
        type_wf,
        npts,
        fs,
        fsr,
        res,
        freq,
        scale,
        phase,
        num_tones
    )
    return c

def gen_tone(c: POINTER(_Config)):
    awf = POINTER(c_double)()
    npts = c_uint(0)
    _gen_tone(c, byref(awf), byref(npts))
    return list(awf[0:npts.value])
'''
def gen_noise(c: POINTER(_Config), size: POINTER(c_uint)):
    noise = POINTER(c_double)()
    _gen_noise(c, size, byref(awf))
    noise_contents = np.frombuffer(noise.contents)
    return noise_contents
'''
def gen_ramp(c: POINTER(_Config)):
    awf = POINTER(c_double)()
    npts = c_uint(0)
    _gen_ramp(c, byref(awf), byref(npts))
    return list(awf[0:npts.value])

def quantize(c: POINTER(_Config), awf: list):
    qwf = POINTER(c_int)()
    awf_ptr = (c_double * len(awf))(*awf)
    _quantize(c, awf_ptr, byref(qwf))
    return list(qwf[0:len(awf)])

def rfft(c: POINTER(_Config), realqwf: list):
    out_i = POINTER(c_double)()
    out_q = POINTER(c_double)()
    realqwf_ptr = (c_int * len(realqwf))(*realqwf)
    fft_size = c_uint(0)
    _rfft(c, realqwf_ptr, byref(out_i), byref(out_q), byref(fft_size))
    out_i_list = list(out_i[0:fft_size.value])
    out_q_list = list(out_q[0:fft_size.value])
    return out_i_list, out_q_list

def fft(c: POINTER(_Config), qwf_i: list, qwf_q: list):
    out_i = POINTER(c_double)()
    out_q = POINTER(c_double)()
    qwf_i_ptr = (c_int * len(qwf_i))(*qwf_i)
    qwf_q_ptr = (c_int * len(qwf_q))(*qwf_i)
    fft_size = c_uint(0)
    _fft(c, qwf_i_ptr, qwf_q_ptr, byref(out_i), byref(out_q), byref(fft_size))
    out_i_list = list(out_i[0:fft_size.value])
    out_q_list = list(out_q[0:fft_size.value])
    return out_i_list, out_q_list
