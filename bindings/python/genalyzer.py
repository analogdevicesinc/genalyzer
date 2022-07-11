from dataclasses import dataclass, field
from typing import List
from ctypes import (
    c_char,
    c_int32,
    c_int64,
    c_uint,
    c_uint64,
    c_ulong,
    c_int,
    c_double,
    c_bool,
    POINTER,
    byref,
    Structure,
    c_void_p,
    c_char_p,
    CDLL,
)
from platform import system as _system
from ctypes.util import find_library
import os

if "Windows" in _system():
    _libgen = "libgenalyzer.dll"
else:
    # Non-windows, possibly Posix system
    _libgen = "genalyzer"

_libgen = CDLL(find_library(_libgen), use_errno=True, use_last_error=True)


class _GNConfig(Structure):
    pass


_GNConfigPtr = POINTER(_GNConfig)


class GNConfig(object):
    """Configuration structure to handle library state"""

    def __init__(self):
        self._struct = _GNConfigPtr()


_gn_config_calloc = _libgen.gn_config_calloc
_gn_config_calloc.restype = c_int
_gn_config_calloc.argtypes = [
    POINTER(_GNConfigPtr),
]

_gn_config_free = _libgen.gn_config_free
_gn_config_free.restype = c_int
_gn_config_free.argtypes = [
    _GNConfigPtr,
]

_gn_config_set_ttype = _libgen.gn_config_set_ttype
_gn_config_set_ttype.restype = c_int
_gn_config_set_ttype.argtypes = [
    c_uint,
    _GNConfigPtr,
]

_gn_config_set_npts = _libgen.gn_config_set_npts
_gn_config_set_npts.restype = c_int
_gn_config_set_npts.argtypes = [
    c_ulong,
    _GNConfigPtr,
]

_gn_config_set_sample_rate = _libgen.gn_config_set_sample_rate
_gn_config_set_sample_rate.restype = c_int
_gn_config_set_sample_rate.argtypes = [
    c_double,
    _GNConfigPtr,
]

_gn_config_set_data_rate = _libgen.gn_config_set_data_rate
_gn_config_set_data_rate.restype = c_int
_gn_config_set_data_rate.argtypes = [
    c_double,
    _GNConfigPtr,
]

_gn_config_set_shift_freq = _libgen.gn_config_set_shift_freq
_gn_config_set_shift_freq.restype = c_int
_gn_config_set_shift_freq.argtypes = [
    c_double,
    _GNConfigPtr,
]

_gn_config_set_num_tones = _libgen.gn_config_set_num_tones
_gn_config_set_num_tones.restype = c_int
_gn_config_set_num_tones.argtypes = [
    c_ulong,
    _GNConfigPtr,
]

_gn_config_set_tone_freq = _libgen.gn_config_set_tone_freq
_gn_config_set_tone_freq.restype = c_int
_gn_config_set_tone_freq.argtypes = [
    POINTER(c_double),
    _GNConfigPtr,
]

_gn_config_set_tone_ampl = _libgen.gn_config_set_tone_ampl
_gn_config_set_tone_ampl.restype = c_int
_gn_config_set_tone_ampl.argtypes = [
    POINTER(c_double),
    _GNConfigPtr,
]

_gn_config_set_tone_phase = _libgen.gn_config_set_tone_phase
_gn_config_set_tone_phase.restype = c_int
_gn_config_set_tone_phase.argtypes = [
    POINTER(c_double),
    _GNConfigPtr,
]

_gn_config_set_fsr = _libgen.gn_config_set_fsr
_gn_config_set_fsr.restype = c_int
_gn_config_set_fsr.argtypes = [
    c_double,
    _GNConfigPtr,
]

_gn_config_set_qres = _libgen.gn_config_set_qres
_gn_config_set_qres.restype = c_int
_gn_config_set_qres.argtypes = [
    c_int,
    _GNConfigPtr,
]

_gn_config_set_noise_rms = _libgen.gn_config_set_noise_rms
_gn_config_set_noise_rms.restype = c_int
_gn_config_set_noise_rms.argtypes = [
    c_double,
    _GNConfigPtr,
]

_gn_config_set_code_format = _libgen.gn_config_set_code_format
_gn_config_set_code_format.restype = c_int
_gn_config_set_code_format.argtypes = [
    c_uint,
    _GNConfigPtr,
]

_gn_config_set_nfft = _libgen.gn_config_set_nfft
_gn_config_set_nfft.restype = c_int
_gn_config_set_nfft.argtypes = [
    c_ulong,
    _GNConfigPtr,
]

_gn_config_set_fft_navg = _libgen.gn_config_set_fft_navg
_gn_config_set_fft_navg.restype = c_int
_gn_config_set_fft_navg.argtypes = [
    c_ulong,
    _GNConfigPtr,
]

_gn_config_set_win = _libgen.gn_config_set_win
_gn_config_set_win.restype = c_int
_gn_config_set_win.argtypes = [
    c_uint,
    _GNConfigPtr,
]

_gn_config_set_ssb_fund = _libgen.gn_config_set_ssb_fund
_gn_config_set_ssb_fund.restype = c_int
_gn_config_set_ssb_fund.argtypes = [
    c_int,
    _GNConfigPtr,
]

_gn_config_set_ssb_rest = _libgen.gn_config_set_ssb_rest
_gn_config_set_ssb_rest.restype = c_int
_gn_config_set_ssb_rest.argtypes = [
    c_int,
    _GNConfigPtr,
]

_gn_config_set_max_harm_order = _libgen.gn_config_set_max_harm_order
_gn_config_set_max_harm_order.restype = c_int
_gn_config_set_max_harm_order.argtypes = [
    c_int,
    _GNConfigPtr,
]

_gn_config_set_dnla_signal_type = _libgen.gn_config_set_dnla_signal_type
_gn_config_set_dnla_signal_type.restype = c_int
_gn_config_set_dnla_signal_type.argtypes = [
    c_uint,
    _GNConfigPtr,
]

_gn_config_set_inla_fit = _libgen.gn_config_set_inla_fit
_gn_config_set_inla_fit.restype = c_int
_gn_config_set_inla_fit.argtypes = [
    c_uint,
    _GNConfigPtr,
]

_gn_config_set_ramp_start = _libgen.gn_config_set_ramp_start
_gn_config_set_ramp_start.restype = c_int
_gn_config_set_ramp_start.argtypes = [
    c_ulong,
    _GNConfigPtr,
]

_gn_config_set_ramp_stop = _libgen.gn_config_set_ramp_stop
_gn_config_set_ramp_stop.restype = c_int
_gn_config_set_ramp_stop.argtypes = [
    c_ulong,
    _GNConfigPtr,
]

_gn_config_gen_tone = _libgen.gn_config_gen_tone
_gn_config_gen_tone.restype = c_int
_gn_config_gen_tone.argtypes = [
    c_uint,
    c_ulong,
    c_double,
    c_ulong,
    POINTER(c_double),
    POINTER(c_double),
    POINTER(c_double),
    _GNConfigPtr,
]

_gn_config_gen_ramp = _libgen.gn_config_gen_ramp
_gn_config_gen_ramp.restype = c_int
_gn_config_gen_ramp.argtypes = [
    c_ulong,
    c_ulong,
    c_ulong,
    _GNConfigPtr,
]

_gn_config_quantize = _libgen.gn_config_quantize
_gn_config_quantize.restype = c_int
_gn_config_quantize.argtypes = [
    c_ulong,
    c_double,
    c_int,
    c_double,
    _GNConfigPtr,
]

_gn_config_histz_nla = _libgen.gn_config_histz_nla
_gn_config_histz_nla.restype = c_int
_gn_config_histz_nla.argtypes = [
    c_ulong,
    c_int,
    _GNConfigPtr,
]

_gn_config_fftz = _libgen.gn_config_fftz
_gn_config_fftz.restype = c_int
_gn_config_fftz.argtypes = [
    c_ulong,
    c_int,
    c_ulong,
    c_ulong,
    c_uint,
    _GNConfigPtr,
]

_gn_config_fa = _libgen.gn_config_fa
_gn_config_fa.restype = c_int
_gn_config_fa.argtypes = [
    c_double,
    _GNConfigPtr,
]

_gn_gen_ramp = _libgen.gn_gen_ramp
_gn_gen_ramp.restype = c_int
_gn_gen_ramp.argtypes = [
    POINTER(POINTER(c_double)),
    _GNConfigPtr,
]

_gn_gen_real_tone = _libgen.gn_gen_real_tone
_gn_gen_real_tone.restype = c_int
_gn_gen_real_tone.argtypes = [
    POINTER(POINTER(c_double)),
    _GNConfigPtr,
]

_gn_gen_complex_tone = _libgen.gn_gen_complex_tone
_gn_gen_complex_tone.restype = c_int
_gn_gen_complex_tone.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(POINTER(c_double)),
    _GNConfigPtr,
]

_gn_quantize = _libgen.gn_quantize
_gn_quantize.restype = c_int
_gn_quantize.argtypes = [
    POINTER(POINTER(c_int32)),
    POINTER(c_double),
    _GNConfigPtr,
]

_gn_fftz = _libgen.gn_fftz
_gn_fftz.restype = c_int
_gn_fftz.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(c_int32),
    POINTER(c_int32),
    _GNConfigPtr,
]

_gn_histz = _libgen.gn_histz
_gn_histz.restype = c_int
_gn_histz.argtypes = [
    POINTER(POINTER(c_uint64)),
    POINTER(c_ulong),
    POINTER(c_int32),
    _GNConfigPtr,
]

_gn_dnlz = _libgen.gn_dnlz
_gn_dnlz.restype = c_int
_gn_dnlz.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_uint64),
    _GNConfigPtr,
]

_gn_inlz = _libgen.gn_inlz
_gn_inlz.restype = c_int
_gn_inlz.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_uint64),
    _GNConfigPtr,
]

_gn_get_wfa_results = _libgen.gn_get_wfa_results
_gn_get_wfa_results.restype = c_int
_gn_get_wfa_results.argtypes = [
    POINTER(POINTER(POINTER(c_char))),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_int32),
    _GNConfigPtr,
]

_gn_get_ha_results = _libgen.gn_get_ha_results
_gn_get_ha_results.restype = c_int
_gn_get_ha_results.argtypes = [
    POINTER(POINTER(POINTER(c_char))),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_uint64),
    _GNConfigPtr,
]

_gn_get_dnla_results = _libgen.gn_get_dnla_results
_gn_get_dnla_results.restype = c_int
_gn_get_dnla_results.argtypes = [
    POINTER(POINTER(POINTER(c_char))),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_double),
    _GNConfigPtr,
]

_gn_get_inla_results = _libgen.gn_get_inla_results
_gn_get_inla_results.restype = c_int
_gn_get_inla_results.argtypes = [
    POINTER(POINTER(POINTER(c_char))),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_double),
    _GNConfigPtr,
]

_gn_get_fa_single_result = _libgen.gn_get_fa_single_result
_gn_get_fa_single_result.restype = c_int
_gn_get_fa_single_result.argtypes = [
    POINTER(c_double),
    POINTER(c_char),
    POINTER(c_double),
    _GNConfigPtr,
]

_gn_get_fa_results = _libgen.gn_get_fa_results
_gn_get_fa_results.restype = c_int
_gn_get_fa_results.argtypes = [
    POINTER(POINTER(POINTER(c_char))),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_double),
    _GNConfigPtr,
]