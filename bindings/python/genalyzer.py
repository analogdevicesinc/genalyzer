'''
* cgenalyzer - genalyzer API header file
*
* Copyright (C) 2022 Analog Devices, Inc.
* Author: Srikanth Pagadarai
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
'''

from dataclasses import dataclass, field
from typing import List
from ctypes import (
    c_char,
    c_uint8,
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
import genalyzer_advanced as advanced

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

_gn_config_free = _libgen.gn_config_free
_gn_config_free.restype = c_int
_gn_config_free.argtypes = [
    POINTER(_GNConfigPtr),
]

_gn_config_set_ttype = _libgen.gn_config_set_ttype
_gn_config_set_ttype.restype = c_int
_gn_config_set_ttype.argtypes = [
    c_uint,
    POINTER(_GNConfigPtr),
]

_gn_config_set_npts = _libgen.gn_config_set_npts
_gn_config_set_npts.restype = c_int
_gn_config_set_npts.argtypes = [
    c_ulong,
    POINTER(_GNConfigPtr),
]

_gn_config_get_npts = _libgen.gn_config_get_npts
_gn_config_get_npts.restype = c_int
_gn_config_get_npts.argtypes = [
    POINTER(c_ulong),
    POINTER(_GNConfigPtr),
]

_gn_config_set_sample_rate = _libgen.gn_config_set_sample_rate
_gn_config_set_sample_rate.restype = c_int
_gn_config_set_sample_rate.argtypes = [
    c_double,
    POINTER(_GNConfigPtr),
]


_gn_config_get_sample_rate = _libgen.gn_config_get_sample_rate
_gn_config_get_sample_rate.restype = c_int
_gn_config_get_sample_rate.argtypes = [
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

_gn_config_set_data_rate = _libgen.gn_config_set_data_rate
_gn_config_set_data_rate.restype = c_int
_gn_config_set_data_rate.argtypes = [
    c_double,
    POINTER(_GNConfigPtr),
]

_gn_config_set_shift_freq = _libgen.gn_config_set_shift_freq
_gn_config_set_shift_freq.restype = c_int
_gn_config_set_shift_freq.argtypes = [
    c_double,
    POINTER(_GNConfigPtr),
]

_gn_config_set_num_tones = _libgen.gn_config_set_num_tones
_gn_config_set_num_tones.restype = c_int
_gn_config_set_num_tones.argtypes = [
    c_ulong,
    POINTER(_GNConfigPtr),
]

_gn_config_set_tone_freq = _libgen.gn_config_set_tone_freq
_gn_config_set_tone_freq.restype = c_int
_gn_config_set_tone_freq.argtypes = [
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

_gn_config_set_tone_ampl = _libgen.gn_config_set_tone_ampl
_gn_config_set_tone_ampl.restype = c_int
_gn_config_set_tone_ampl.argtypes = [
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

_gn_config_set_tone_phase = _libgen.gn_config_set_tone_phase
_gn_config_set_tone_phase.restype = c_int
_gn_config_set_tone_phase.argtypes = [
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

_gn_config_set_fsr = _libgen.gn_config_set_fsr
_gn_config_set_fsr.restype = c_int
_gn_config_set_fsr.argtypes = [
    c_double,
    POINTER(_GNConfigPtr),
]

_gn_config_set_qres = _libgen.gn_config_set_qres
_gn_config_set_qres.restype = c_int
_gn_config_set_qres.argtypes = [
    c_int,
    POINTER(_GNConfigPtr),
]

_gn_config_set_noise_rms = _libgen.gn_config_set_noise_rms
_gn_config_set_noise_rms.restype = c_int
_gn_config_set_noise_rms.argtypes = [
    c_double,
    POINTER(_GNConfigPtr),
]

_gn_config_set_code_format = _libgen.gn_config_set_code_format
_gn_config_set_code_format.restype = c_int
_gn_config_set_code_format.argtypes = [
    c_uint,
    POINTER(_GNConfigPtr),
]

_gn_config_set_nfft = _libgen.gn_config_set_nfft
_gn_config_set_nfft.restype = c_int
_gn_config_set_nfft.argtypes = [
    c_ulong,
    POINTER(_GNConfigPtr),
]

_gn_config_get_nfft = _libgen.gn_config_get_nfft
_gn_config_get_nfft.restype = c_int
_gn_config_get_nfft.argtypes = [
    POINTER(c_ulong),
    POINTER(_GNConfigPtr),
]

_gn_config_set_fft_navg = _libgen.gn_config_set_fft_navg
_gn_config_set_fft_navg.restype = c_int
_gn_config_set_fft_navg.argtypes = [
    c_ulong,
    POINTER(_GNConfigPtr),
]

_gn_config_set_win = _libgen.gn_config_set_win
_gn_config_set_win.restype = c_int
_gn_config_set_win.argtypes = [
    c_uint,
    POINTER(_GNConfigPtr),
]

_gn_config_set_ssb_fund = _libgen.gn_config_set_ssb_fund
_gn_config_set_ssb_fund.restype = c_int
_gn_config_set_ssb_fund.argtypes = [
    c_int,
    POINTER(_GNConfigPtr),
]

_gn_config_set_ssb_rest = _libgen.gn_config_set_ssb_rest
_gn_config_set_ssb_rest.restype = c_int
_gn_config_set_ssb_rest.argtypes = [
    c_int,
    POINTER(_GNConfigPtr),
]

_gn_config_set_max_harm_order = _libgen.gn_config_set_max_harm_order
_gn_config_set_max_harm_order.restype = c_int
_gn_config_set_max_harm_order.argtypes = [
    c_int,
    POINTER(_GNConfigPtr),
]

_gn_config_set_dnla_signal_type = _libgen.gn_config_set_dnla_signal_type
_gn_config_set_dnla_signal_type.restype = c_int
_gn_config_set_dnla_signal_type.argtypes = [
    c_uint,
    POINTER(_GNConfigPtr),
]

_gn_config_set_inla_fit = _libgen.gn_config_set_inla_fit
_gn_config_set_inla_fit.restype = c_int
_gn_config_set_inla_fit.argtypes = [
    c_uint,
    POINTER(_GNConfigPtr),
]

_gn_config_set_ramp_start = _libgen.gn_config_set_ramp_start
_gn_config_set_ramp_start.restype = c_int
_gn_config_set_ramp_start.argtypes = [
    c_ulong,
    POINTER(_GNConfigPtr),
]

_gn_config_set_ramp_stop = _libgen.gn_config_set_ramp_stop
_gn_config_set_ramp_stop.restype = c_int
_gn_config_set_ramp_stop.argtypes = [
    c_ulong,
    POINTER(_GNConfigPtr),
]

_gn_config_get_code_density_size = _libgen.gn_config_get_code_density_size
_gn_config_get_code_density_size.restype = c_int
_gn_config_get_code_density_size.argtypes = [
    POINTER(c_ulong),
    POINTER(_GNConfigPtr),
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
    POINTER(_GNConfigPtr),
]

_gn_config_gen_ramp = _libgen.gn_config_gen_ramp
_gn_config_gen_ramp.restype = c_int
_gn_config_gen_ramp.argtypes = [
    c_ulong,
    c_ulong,
    c_ulong,
    POINTER(_GNConfigPtr),
]

_gn_config_quantize = _libgen.gn_config_quantize
_gn_config_quantize.restype = c_int
_gn_config_quantize.argtypes = [
    c_ulong,
    c_double,
    c_int,
    c_double,
    POINTER(_GNConfigPtr),
]

_gn_config_histz_nla = _libgen.gn_config_histz_nla
_gn_config_histz_nla.restype = c_int
_gn_config_histz_nla.argtypes = [
    c_ulong,
    c_int,
    POINTER(_GNConfigPtr),
]

_gn_config_fftz = _libgen.gn_config_fftz
_gn_config_fftz.restype = c_int
_gn_config_fftz.argtypes = [
    c_ulong,
    c_int,
    c_ulong,
    c_ulong,
    c_uint,
    POINTER(_GNConfigPtr),
]

_gn_config_fa = _libgen.gn_config_fa
_gn_config_fa.restype = c_int
_gn_config_fa.argtypes = [
    c_double,
    POINTER(_GNConfigPtr),
]

_gn_config_fa_auto = _libgen.gn_config_fa_auto
_gn_config_fa_auto.restype = c_int
_gn_config_fa_auto.argtypes = [
    c_uint8,
    POINTER(_GNConfigPtr),
]

_gn_gen_ramp = _libgen.gn_gen_ramp
_gn_gen_ramp.restype = c_int
_gn_gen_ramp.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(_GNConfigPtr),
]

_gn_gen_real_tone = _libgen.gn_gen_real_tone
_gn_gen_real_tone.restype = c_int
_gn_gen_real_tone.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(_GNConfigPtr),
]

_gn_gen_complex_tone = _libgen.gn_gen_complex_tone
_gn_gen_complex_tone.restype = c_int
_gn_gen_complex_tone.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(POINTER(c_double)),
    POINTER(_GNConfigPtr),
]

_gn_quantize = _libgen.gn_quantize
_gn_quantize.restype = c_int
_gn_quantize.argtypes = [
    POINTER(POINTER(c_int32)),
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

_gn_fftz = _libgen.gn_fftz
_gn_fftz.restype = c_int
_gn_fftz.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(c_int32),
    POINTER(c_int32),
    POINTER(_GNConfigPtr),
]

_gn_histz = _libgen.gn_histz
_gn_histz.restype = c_int
_gn_histz.argtypes = [
    POINTER(POINTER(c_uint64)),
    POINTER(c_ulong),
    POINTER(c_int32),
    POINTER(_GNConfigPtr),
]

_gn_dnlz = _libgen.gn_dnlz
_gn_dnlz.restype = c_int
_gn_dnlz.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_uint64),
    POINTER(_GNConfigPtr),
]

_gn_inlz = _libgen.gn_inlz
_gn_inlz.restype = c_int
_gn_inlz.argtypes = [
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_uint64),
    POINTER(_GNConfigPtr),
]

_gn_get_wfa_results = _libgen.gn_get_wfa_results
_gn_get_wfa_results.restype = c_int
_gn_get_wfa_results.argtypes = [
    POINTER(POINTER(c_char_p)),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_int32),
    POINTER(_GNConfigPtr),
]

_gn_get_ha_results = _libgen.gn_get_ha_results
_gn_get_ha_results.restype = c_int
_gn_get_ha_results.argtypes = [
    POINTER(POINTER(c_char_p)),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_uint64),
    POINTER(_GNConfigPtr),
]

_gn_get_dnla_results = _libgen.gn_get_dnla_results
_gn_get_dnla_results.restype = c_int
_gn_get_dnla_results.argtypes = [
    POINTER(POINTER(c_char_p)),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

_gn_get_inla_results = _libgen.gn_get_inla_results
_gn_get_inla_results.restype = c_int
_gn_get_inla_results.argtypes = [
    POINTER(POINTER(c_char_p)),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

_gn_get_fa_single_result = _libgen.gn_get_fa_single_result
_gn_get_fa_single_result.restype = c_int
_gn_get_fa_single_result.argtypes = [
    POINTER(c_double),
    c_char_p,
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

_gn_get_fa_results = _libgen.gn_get_fa_results
_gn_get_fa_results.restype = c_int
_gn_get_fa_results.argtypes = [
    POINTER(POINTER(c_char_p)),
    POINTER(POINTER(c_double)),
    POINTER(c_ulong),
    POINTER(c_double),
    POINTER(_GNConfigPtr),
]

def config_free(
    c: GNConfig,
) -> int:
    _gn_config_free(
        byref(c._struct)
    )


def config_gen_ramp(
    npts: int,
    ramp_start: int,
    ramp_stop: int,
    *args
) -> GNConfig:
    """Configure GNConfig struct to generate tone.
    :param npts: number of sample points in the waveform
    :param ramp_start: Input start value of ramp
    :param ramp_stop: Input stop value of ramp
    :return: GNConfig object
    """
    if len(args) == 0:
        c = GNConfig()
    elif len(args) == 1:
        c = args[0]
    npts = c_ulong(npts)
    ramp_start = c_ulong(ramp_start)
    ramp_stop = c_ulong(ramp_stop)

    _gn_config_gen_ramp(
        npts,
        ramp_start,
        ramp_stop,
        byref(c._struct)
    )
    return c

def config_gen_tone(
    ttype: int,
    npts: int,
    sample_rate: float,
    num_tones: int,
    tone_freq: float,
    tone_ampl: float,
    tone_phase: float,
    *args
) -> GNConfig:
    """Configure GNConfig struct to generate tone.
    :param ttype: tone type
    :param npts: number of sample points in the waveform
    :param sample_rate: sampling frequency
    :param num_tones: number of tones
    :param tone_freq: tone frequency
    :param tone_ampl: tone amplitude
    :param tone_phase: tone phase
    :return: GNConfig object
    """
    if len(args) == 0:
        c = GNConfig()
    elif len(args) == 1:
        c = args[0]
    ttype = c_uint(ttype)
    npts = c_ulong(npts)
    sample_rate = c_double(sample_rate)
    num_tones = c_ulong(num_tones)
    double_array = c_double * num_tones.value
    tone_freq = (double_array)(*tone_freq)
    tone_ampl = (double_array)(*tone_ampl)
    tone_phase = (double_array)(*tone_phase)    

    _gn_config_gen_tone(
        ttype,
        npts,
        sample_rate,
        num_tones,
        tone_freq,
        tone_ampl,
        tone_phase,
        byref(c._struct)
    )
    return c


def config_quantize(
    npts: int,
    fsr: float,
    qres: int,
    qnoise: float,
    *args
) -> GNConfig:
    """Configure GNConfig struct to perform quantization.
    :param npts: number of sample points in the waveform
    :param fsr: full-scale range
    :param qres: quantization resolution
    :param qnoise: quantization noise
    :return: GNConfig object
    """    
    if len(args) == 0:
        c = GNConfig()
    elif len(args) == 1:
        c = args[0]

    npts = c_ulong(npts)
    fsr = c_double(fsr)
    qres = c_int(qres)
    qnoise = c_double(qnoise)

    _gn_config_quantize(
        npts,
        fsr,
        qres,
        qnoise,
        byref(c._struct)
    )
    return c

def config_histz_nla(
    npts: int,
    qres: int,
    *args
) -> GNConfig:
    """Configure GNConfig struct to compute histogram or perform non-linearity analysis.
    :param npts: number of sample points in the waveform
    :param fsr: full-scale range
    :param qres: quantization resolution
    :param qnoise: quantization noise
    :return: GNConfig object
    """    
    if len(args) == 0:
        c = GNConfig()
    elif len(args) == 1:
        c = args[0]

    npts = c_ulong(npts)
    qres = c_int(qres)
    
    _gn_config_histz_nla(
        npts,
        qres,
        byref(c._struct)
    )
    return c

def config_fftz(
    npts: int,
    qres: int,
    navg: int,
    nfft:int,
    win:int,
    *args
) -> GNConfig:
    """Configure GNConfig struct to compute FFT.
    :param npts: number of sample points in the waveform
    :param fsr: full-scale range
    :param qres: quantization resolution
    :param qnoise: quantization noise
    :return: GNConfig object
    """    
    if len(args) == 0:
        c = GNConfig()
    elif len(args) == 1:
        c = args[0]
    
    npts = c_ulong(npts)
    qres = c_int(qres)
    navg = c_ulong(navg)
    nfft = c_ulong(nfft)
    win = c_uint(win)

    _gn_config_fftz(
        npts,
        qres,
        navg,
        nfft,
        win,
        byref(c._struct)
    )
    return c

def config_fa(
    fixed_tone_freq: float,
    *args
) -> GNConfig:
    """Configure GNConfig struct for Fourier analysis.
    :param fixed_tone_freq: fixed tone frequency
    :return: GNConfig object
    """    
    if len(args) == 0:
        c = GNConfig()
    elif len(args) == 1:
        c = args[0]
    
    fixed_tone_freq = c_double(fixed_tone_freq)
    
    _gn_config_fa(
        fixed_tone_freq, 
        byref(c._struct)
    )
    return c

def gn_config_fa_auto(ssb_width: int, c: GNConfig):
    """Configure GNConfig struct for Fourier analysis where tones are
    automatically found.
    :param ssb_width: SSB width
    :param c: GNConfig object
    """
    ssb_width = c_uint8(ssb_width)

    ret = _gn_config_fa_auto(
        ssb_width,
        byref(c._struct)
    )
    if ret != 0:
        raise Exception("gn_config_fa_auto failed")

def gen_ramp(
    c: GNConfig
) -> List[float]:
    """Generate floating-point ramp waveform
    :param c: GNConfig object
    :return: real ramp waveform as list of floats
    """
    awf = POINTER(c_double)()
    wf_len = c_ulong(0)
    _gn_config_get_npts(byref(wf_len), byref(c._struct))
    _gn_gen_ramp(byref(awf), byref(c._struct))
    return list(awf[0 : wf_len.value])


def gen_real_tone(
    c: GNConfig
) -> List[float]:
    """Generate single-tone or multi-tone floating-point waveform
    :param c: GNConfig object
    :return: single-/multi-tone real waveform as list of floats
    """
    awf = POINTER(c_double)()
    wf_len = c_ulong(0)
    _gn_config_get_npts(byref(wf_len), byref(c._struct))
    _gn_gen_real_tone(byref(awf), byref(c._struct))
    return list(awf[0 : wf_len.value])

def gen_complex_tone(
    c: GNConfig
) -> List[float]:
    """Generate single-tone or multi-tone complex waveform
    :param c: GNConfig object
    :return: single-/multi-tone complex waveform as (I and Q) lists of floats
    """
    awf_i = POINTER(c_double)()
    awf_q = POINTER(c_double)()
    wf_len = c_ulong(0)
    _gn_config_get_npts(byref(wf_len), byref(c._struct))
    _gn_gen_complex_tone(byref(awf_i), byref(awf_q), byref(c._struct))
    return (list(awf_i[0 : wf_len.value]), list(awf_q[0 : wf_len.value]))

def quantize(
    in_awf: list,
    c: GNConfig
) -> List[int]:
    """Quantize real waveform
    :param c: GNConfig object
    :return: Quantized floating-point waveform as list of ints
    """
    qwf = POINTER(c_int32)()
    wf_len = c_ulong(0)
    _gn_config_get_npts(byref(wf_len), byref(c._struct))
    double_array = c_double * wf_len.value
    in_awf_ptr = (double_array)(*in_awf)
    _gn_quantize(byref(qwf), in_awf_ptr, byref(c._struct))
    return list(qwf[0 : wf_len.value])

def fftz(
    in_qwfi: int,
    in_qwfq: int,
    c: GNConfig
) -> List[float]:
    """Compute FFT
    :param in_qwfi: Input quantized waveform (I)
    :param in_qwfq: Input quantized waveform (Q)
    :param c: GNConfig object
    :return: FFT of the input waveform as list of floats
    """
    fft_out = POINTER(c_double)()
    wf_len = c_ulong(0)
    _gn_config_get_npts(byref(wf_len), byref(c._struct))
    int32_array = c_int32 * wf_len.value
    in_qwfi = (int32_array)(*in_qwfi)
    in_qwfq = (int32_array)(*in_qwfq)
    _gn_fftz(byref(fft_out), in_qwfi, in_qwfq, byref(c._struct))
    fft_len = c_ulong(0)
    _gn_config_get_nfft(byref(fft_len), byref(c._struct))
    out = list(fft_out[0 : 2*fft_len.value])
    fft_out_i = [out[i] for i in range(len(out)) if i % 2 == 0]
    fft_out_q = [out[i] for i in range(len(out)) if i % 2 != 0]
    return fft_out_i, fft_out_q

def histz(
    in_qwf: int,
    c: GNConfig
) -> List[int]:
    """Compute histogram
    :param in_qwf: Input quantized waveform
    :param c: GNConfig object
    :return: Histogram of the input waveform as list of floats
    """
    hist_out = POINTER(c_uint64)()
    wf_len = c_ulong(0)
    _gn_config_get_npts(byref(wf_len), byref(c._struct))
    int32_array = c_int32 * wf_len.value
    in_qwf = (int32_array)(*in_qwf)
    hist_len = c_ulong(0)
    _gn_histz(byref(hist_out), byref(hist_len), in_qwf, byref(c._struct))    
    return list(hist_out[0 : hist_len.value])

def get_ha_results(
    hist_in: int,
    c: GNConfig
) -> dict:
    """Get Fourier analysis results.
    :param hist_in: Input histogram data
    :param: GNConfig object
    :return: Results as dict
    """
    cd_len = c_ulong(0)
    _gn_config_get_code_density_size(byref(cd_len), byref(c._struct))
    uint64_array = c_uint64 * cd_len.value
    hist_in = (uint64_array)(*hist_in)
    rkeys = POINTER(c_char_p)()
    rvalues = POINTER(c_double)()
    results_size = c_ulong(0)
    _gn_get_ha_results(byref(rkeys), byref(rvalues), byref(results_size), hist_in, byref(c._struct))
    ha_results = dict()
    for i in range(results_size.value):
        ha_results[(rkeys[i]).decode('ascii')] = rvalues[i]
    return ha_results

def get_wfa_results(
    in_qwf: int,
    c: GNConfig
) -> dict:
    """Get Fourier analysis results.
    :param hist_in: Input histogram data
    :param: GNConfig object
    :return: Results as dict
    """
    wf_len = c_ulong(0)
    _gn_config_get_npts(byref(wf_len), byref(c._struct))
    int32_array = c_int32 * wf_len.value
    in_qwf = (int32_array)(*in_qwf)
    rkeys = POINTER(c_char_p)()
    rvalues = POINTER(c_double)()
    results_size = c_ulong(0)
    _gn_get_wfa_results(byref(rkeys), byref(rvalues), byref(results_size), in_qwf, byref(c._struct))
    wfa_results = dict()
    for i in range(results_size.value):
        wfa_results[(rkeys[i]).decode('ascii')] = rvalues[i]
    return wfa_results

def get_fa_single_result(
    metric_name: str,
    fft_ilv: float,
    c: GNConfig
) -> float:
    """Get Fourier analysis results.
    :param fixed_tone_freq: fixed tone frequency
    :param: GNConfig object
    :return: Results as dict
    """
    fft_len = c_ulong(0)
    _gn_config_get_nfft(byref(fft_len), byref(c._struct))
    fft_ilv_len = 2 * fft_len.value
    double_array = c_double * fft_ilv_len
    fft_ilv = (double_array)(*fft_ilv)
    metric_name_enc = metric_name.encode("utf-8")
    result = c_double(0)
    _gn_get_fa_single_result(byref(result), metric_name_enc, fft_ilv, byref(c._struct))
    return result.value

def get_fa_results(
    fft_ilv: float,
    c: GNConfig
) -> dict:
    """Get Fourier analysis results.
    :param fixed_tone_freq: fixed tone frequency
    :param: GNConfig object
    :return: Results as dict
    """
    fft_len = c_ulong(0)
    _gn_config_get_nfft(byref(fft_len), byref(c._struct))
    fft_ilv_len = 2 * fft_len.value
    double_array = c_double * fft_ilv_len
    fft_ilv = (double_array)(*fft_ilv)
    rkeys = POINTER(c_char_p)()
    rvalues = POINTER(c_double)()
    results_size = c_ulong(0)
    _gn_get_fa_results(byref(rkeys), byref(rvalues), byref(results_size), fft_ilv, byref(c._struct))
    fa_results = dict()
    for i in range(results_size.value):
        fa_results[(rkeys[i]).decode('ascii')] = rvalues[i]
    return fa_results

def config_set_sample_rate(
    sample_rate: float,
    c: GNConfig
) -> None:
    """Set sample rate.
    :param sample_rate: Sample rate in Hz
    :param c: GNConfig object
    """
    _gn_config_set_sample_rate(sample_rate, byref(c._struct))

def config_code_format(
    code_format: int,
    c: GNConfig
) -> None:
    """Configure code format.
    :param code_format: code format (Offset binary, Twos complement)
    :param c: GNConfig object
    """
    code_format = c_uint(code_format)
    _gn_config_set_code_format(code_format, byref(c._struct))