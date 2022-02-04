from dataclasses import dataclass, field
from typing import List
from ctypes import (
    c_uint,
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


_gn_config_tone_gen = _libgen.gn_config_tone_gen
_gn_config_tone_gen.restype = None
_gn_config_tone_gen.argtypes = [
    POINTER(_GNConfigPtr),
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
    c_uint,
    c_bool,
    c_bool,
    c_bool,
]

_gn_config_tone_meas = _libgen.gn_config_tone_meas
_gn_config_tone_meas.restype = None
_gn_config_tone_meas.argtypes = [
    POINTER(_GNConfigPtr),
    c_uint,
    c_uint,
    c_ulong,
    c_int,
    c_double,
    c_double,
    c_int,
    c_uint,
    c_bool,
    c_bool,
    c_bool,
]

_gn_config_noise_meas = _libgen.gn_config_noise_meas
_gn_config_noise_meas.restype = None
_gn_config_noise_meas.argtypes = [
    POINTER(_GNConfigPtr),
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


_gn_config_tone_nl_meas = _libgen.gn_config_tone_nl_meas
_gn_config_tone_nl_meas.restype = None
_gn_config_tone_nl_meas.argtypes = [
    POINTER(_GNConfigPtr),
    c_uint,
    c_ulong,
    c_double,
    c_double,
    c_int,
    POINTER(c_double),
    POINTER(c_double),
    POINTER(c_double),
    c_ulong,
]

_gn_config_ramp_nl_meas = _libgen.gn_config_ramp_nl_meas
_gn_config_ramp_nl_meas.restype = None
_gn_config_ramp_nl_meas.argtypes = [
    POINTER(_GNConfigPtr),
    c_ulong,
    c_double,
    c_double,
    c_int,
    c_double,
    c_double,
    c_double,
]

_gn_gen_tone = _libgen.gn_gen_tone
_gn_gen_tone.restype = None
_gn_gen_tone.argtypes = [
    _GNConfigPtr,
    POINTER(POINTER(c_double)),
    POINTER(c_uint),
]

_gn_gen_noise = _libgen.gn_gen_noise
_gn_gen_noise.restype = None
_gn_gen_noise.argtypes = [
    _GNConfigPtr,
    POINTER(c_uint),
    POINTER(POINTER(c_double)),
]

_gn_gen_ramp = _libgen.gn_gen_ramp
_gn_gen_ramp.restype = None
_gn_gen_ramp.argtypes = [_GNConfigPtr, POINTER(POINTER(c_double)), POINTER(c_uint)]

_gn_quantize = _libgen.gn_quantize
_gn_quantize.restype = None
_gn_quantize.argtypes = [
    _GNConfigPtr,
    POINTER(c_double),
    POINTER(POINTER(c_int)),
]

_gn_rfft = _libgen.gn_rfft
_gn_rfft.restype = None
_gn_rfft.argtypes = [
    _GNConfigPtr,
    POINTER(c_int),
    POINTER(POINTER(c_double)),
    POINTER(POINTER(c_double)),
    POINTER(c_uint),
]

_gn_fft = _libgen.gn_fft
_gn_fft.restype = None
_gn_fft.argtypes = [
    _GNConfigPtr,
    POINTER(c_int),
    POINTER(c_int),
    POINTER(POINTER(c_double)),
    POINTER(POINTER(c_double)),
    POINTER(c_uint),
]

_gn_metric = _libgen.gn_metric
_gn_metric.restype = c_double
_gn_metric.argtypes = [
    _GNConfigPtr,
    c_void_p,
    c_char_p,
    POINTER(POINTER(c_double)),
    POINTER(POINTER(c_double)),
    POINTER(c_uint),
    POINTER(c_uint),
]


@dataclass
class gn_params:
    """
    A dataclass to contain measurement parameters.
    Attributes
    ----------
    fs: float
        Sample-rate
    fsr: float
        Full-scale range
    freq: list[float]
        List of tone frequencies
    phase: list[float]
        List of tone phases
    scale: list[float]
        List of tone scales
    start: float
        Starting value of ramp waveform
    stop: float
        Ending value of ramp waveform
    domain_wf : int
        Waveform domain. Time - 0; FREQ - 1
    type_wf: int
        Waveform type. Real Cosine - 0; Real Sine - 1; Complex Exp - 2
    nfft: int
        FFT order
    navg: int
        Num. of FFTs averaged
    npts: int
        Num. of data points to generate
    num_tones: int
        Num. of tones
    res: int
        Resolution
    window: int
        Window function applied.  0 - BlackmanHarris; 1 - Hanning; 2 - Rectangular
    fsample_update: bool
        Update fsample
    fdata_update: bool
        Update fdata
    fshift_update: bool
        Update fshift
    """

    fs: float
    fsr: float
    freq: List[float] = field(default_factory = list)
    phase: List[float] = field(default_factory = list)
    scale: List[float] = field(default_factory = list)
    start: float = 0.0
    stop: float = 0.1
    domain_wf: int = 0
    type_wf: int = 0
    nfft: int = 1024
    navg: int = 1
    npts: int = 1024
    num_tones: int = 1
    res: int = 0
    win: int = 1
    fsample_update: bool = False
    fdata_update: bool = False
    fshift_update: bool = False


def config_tone_meas(p: gn_params) -> GNConfig:
    """Configure tone generation for tone-based test

    :param p: object of gn_params dataclass
    :return: GNConfig object
    """
    c = GNConfig()
    domain_wf = c_uint(p.domain_wf)
    type_wf = c_uint(p.type_wf)
    nfft = c_ulong(p.nfft)
    navg = c_int(p.navg)
    fs = c_double(p.fs)
    fsr = c_double(p.fsr)
    res = c_int(p.res)
    assert len(p.freq) == len(
        p.phase
    ), "number of frequency values need to match number of phase values"
    assert len(p.phase) == len(
        p.scale
    ), "number of phase values need to match number of scale values"
    num_tones = c_ulong(len(p.freq))
    double_array = c_double * num_tones.value
    freq = (double_array)(*p.freq)
    phase = (double_array)(*p.phase)
    scale = (double_array)(*p.scale)
    win = c_uint(p.win)
    fsample_update = c_bool(False)
    fdata_update = c_bool(False)
    fshift_update = c_bool(False)

    _gn_config_tone_gen(
        byref(c._struct),
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
        win,
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
) -> GNConfig:
    """Configure measurement for noise-based test.

    :param type_wf: waveform type
    :param nfft: number of points in FFT
    :param navg: number of averages
    :param fs: sampling frequency
    :param fsr: sampling frequency resolution
    :param res: resolution
    :param npr: noise power ratio
    :param fsample_update: update sampling frequency
    :param fdata_update: update data
    :param fshift_update: update shift
    :return: GNConfig object
    """
    c = GNConfig()
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

    _gn_config_noise_meas(
        byref(c._struct),
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


def config_ramp_nl_meas(p: gn_params) -> GNConfig:
    """Configure measurement for ramp-based test

    :param p: object of gn_params dataclass
    :return: GNConfig object
    """
    c = GNConfig()
    npts = c_ulong(p.npts)
    fs = c_double(p.fs)
    fsr = c_double(p.fsr)
    res = c_int(0)
    assert (
        p.start < p.stop
    ), "ramp waveform start value needs to be less than ramp stop value"
    start = c_double(p.start)
    stop = c_double(p.stop)

    _gn_config_ramp_nl_meas(byref(c._struct), npts, fs, fsr, res, start, stop, 0.0)
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
) -> GNConfig:
    """Configure measurement for non-linear tone-based test

    :param type_wf: waveform type
    :param npts: number of points in FFT
    :param fs: sampling frequency
    :param fsr: sampling frequency resolution
    :param res: resolution
    :param num_tones: number of tones
    :param freq: frequency values
    :param phase: phase values
    :param scale: scale values
    :return: GNConfig object
    """
    c = GNConfig()
    npts = c_ulong(npts)
    fs = c_double(fs)
    fsr = c_double(fsr)
    res = c_int(0)
    num_tones = c_ulong(num_tones)
    double_array = c_double * num_tones.value
    freq = double_array(*freq)
    phase = double_array(*phase)
    scale = double_array(*scale)

    _gn_config_tone_nl_meas(
        byref(c._struct), type_wf, npts, fs, fsr, res, freq, scale, phase, num_tones
    )
    return c


def gen_tone(c: GNConfig) -> List[float]:
    """Generate single-tone or multi-tone waveform

    :param c: GNConfig object
    :return: single-/multi-tone floating-point waveform as list
    """
    awf = POINTER(c_double)()
    npts = c_uint(0)
    _gn_gen_tone(c._struct, byref(awf), byref(npts))
    return list(awf[0 : npts.value])


def gen_ramp(c: GNConfig) -> List[float]:
    """Generate ramp waveform:

    :param c: GNConfig object
    :return: floating-point ramp waveform as list
    """
    awf = POINTER(c_double)()
    npts = c_uint(0)
    _gn_gen_ramp(c._struct, byref(awf), byref(npts))
    return list(awf[0 : npts.value])


def quantize(c: GNConfig, awf: list) -> List[int]:
    """Quantize single-tone or multi-tone waveform:

    :param c: GNConfig object
    :param awf: floating-point waveform as list
    :return: quantized waveform as list
    """
    qwf = POINTER(c_int)()
    awf_ptr = (c_double * len(awf))(*awf)
    _gn_quantize(c._struct, awf_ptr, byref(qwf))
    return list(qwf[0 : len(awf)])


def rfft(c: GNConfig, realqwf: list):
    """Compute FFT of a real waveform:

    :param c: GNConfig object
    :param realqwf: real waveform
    :return: floating-point I and Q of FFT as lists
    """
    out_i = POINTER(c_double)()
    out_q = POINTER(c_double)()
    realqwf_ptr = (c_int * len(realqwf))(*realqwf)
    fft_size = c_uint(0)
    _gn_rfft(c._struct, realqwf_ptr, byref(out_i), byref(out_q), byref(fft_size))
    out_i_list = list(out_i[0 : fft_size.value])
    out_q_list = list(out_q[0 : fft_size.value])

    return out_i_list, out_q_list


def fft(c: GNConfig, qwf_i: list, qwf_q: list):
    """Compute FFT of a complex waveform:

    :param c: GNConfig object
    :param qwf_i: real part of complex waveform
    :param qwf_q: imaginary part of complex waveform
    :return: floating-point I and Q of FFT as lists
    """
    out_i = POINTER(c_double)()
    out_q = POINTER(c_double)()
    qwf_i_ptr = (c_int * len(qwf_i))(*qwf_i)
    qwf_q_ptr = (c_int * len(qwf_q))(*qwf_i)
    fft_size = c_uint(0)
    _gn_fft(
        c._struct, qwf_i_ptr, qwf_q_ptr, byref(out_i), byref(out_q), byref(fft_size)
    )
    out_i_list = list(out_i[0 : fft_size.value])
    out_q_list = list(out_q[0 : fft_size.value])
    return out_i_list, out_q_list


def metric_t(c: GNConfig, qwf: list, m_name: str) -> float:
    """Compute desired performance metric based on time domain data.

    :param c: opaque configuration struct corresponding to the measurement desired
    :param qwf: quantized waveform
    :param m_name: name of the metric to compute
    :return: computed metric, floating-point I and Q of computed FFT as lists, error code returned
    """
    qwf = qwf.astype(int)
    qwf_ptr = (c_int * len(qwf))(*qwf)
    m_name_enc = m_name.encode("utf-8")
    r = c_double(0.0)
    fft_i = POINTER(c_double)()
    fft_q = POINTER(c_double)()
    fft_size = c_uint(0)
    err_code = c_uint(0)
    r = _gn_metric(
        c._struct,
        qwf_ptr,
        m_name_enc,
        byref(fft_i),
        byref(fft_q),
        byref(fft_size),
        byref(err_code),
    )

    if err_code.value != 0:
        raise Exception(f"Failed to get metric. ERROR: {err_code.value}")

    fft_i_list = list(fft_i[0 : fft_size.value])
    fft_q_list = list(fft_q[0 : fft_size.value])

    return r, fft_i_list, fft_q_list, err_code


def metric_f(c: GNConfig, fft: list, m_name: str) -> float:
    """Compute desired performance metric based on frequency domain data.

    :param c: opaque configuration struct corresponding to the measurement desired
    :param fft: FFT of quantized waveform
    :param m_name: name of the metric to compute
    :return: computed metric, error code returned
    """
    fft_ptr = (c_double * len(fft))(*fft)
    m_name_enc = m_name.encode("utf-8")
    r = c_double(0.0)
    fft_i = POINTER(c_double)()
    fft_q = POINTER(c_double)()
    fft_size = c_uint(0)
    err_code = c_uint(0)
    r = _gn_metric(
        c._struct,
        fft_ptr,
        m_name_enc,
        byref(fft_i),
        byref(fft_q),
        byref(fft_size),
        byref(err_code),
    )

    if err_code.value != 0:
        raise Exception(f"Failed to get metric. ERROR: {err_code.value}")

    return r, err_code
