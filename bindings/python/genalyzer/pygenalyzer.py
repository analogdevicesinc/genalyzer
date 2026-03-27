# Copyright (C) 2024-2026 Analog Devices, Inc.
#
# SPDX short identifier: ADIBSD OR GPL-2.0-or-later

"""
Python wrapper for Genalyzer Library (genalyzer_plus_plus)
"""

import ctypes as _ctypes
import os as _os
import sys as _sys
from ctypes.util import find_library as _find_library
from enum import IntEnum as _IntEnum

import numpy as _np
from numpy.ctypeslib import ndpointer as _ndptr

_c_bool = _ctypes.c_bool
_c_char_p = _ctypes.c_char_p
_c_double = _ctypes.c_double
_c_int = _ctypes.c_int
_c_int16 = _ctypes.c_int16
_c_int32 = _ctypes.c_int32
_c_int64 = _ctypes.c_int64
_c_size_t = _ctypes.c_size_t

_c_bool_p = _ctypes.POINTER(_c_bool)
_c_char_p_p = _ctypes.POINTER(_c_char_p)
_c_double_p = _ctypes.POINTER(_c_double)
_c_size_t_p = _ctypes.POINTER(_c_size_t)

_ndptr_f64_1d = _ndptr(dtype=_np.float64, ndim=1)
_ndptr_i16_1d = _ndptr(dtype=_np.int16, ndim=1)
_ndptr_i32_1d = _ndptr(dtype=_np.int32, ndim=1)
_ndptr_i64_1d = _ndptr(dtype=_np.int64, ndim=1)
_ndptr_u16_1d = _ndptr(dtype=_np.uint16, ndim=1)
_ndptr_u32_1d = _ndptr(dtype=_np.uint32, ndim=1)
_ndptr_u64_1d = _ndptr(dtype=_np.uint64, ndim=1)
del _ndptr

_module_dir = _os.path.dirname(__file__)
if "linux" == _sys.platform:
    _libpath = _find_library("genalyzer")
elif "win32" == _sys.platform:
    _libpath = _find_library("libgenalyzer.dll")
else:
    raise Exception(f"Platform '{_sys.platform}' is not supported.")

if _libpath is None:
    raise OSError(2, "Could not find genalyzer C library")
_lib = _ctypes.cdll.LoadLibrary(_libpath)

del _find_library, _os, _sys

_lib.gn_set_string_termination(True)

"""
Wrapper Utilities
"""

_lib.gn_analysis_results_key_sizes.argtypes = [_c_size_t_p, _c_size_t, _c_int]
_lib.gn_analysis_results_size.argtypes = [_c_size_t_p, _c_int]


def _check_ndarray(a, dtype):
    if not isinstance(a, _np.ndarray):
        raise TypeError(f"Expected numpy.ndarray, got {type(a).__name__}")
    if type(dtype) in [list, tuple]:
        if a.dtype not in dtype:
            raise TypeError(
                "Expected dtype in [{}], got {}".format(", ".join(dtype), a.dtype)
            )
    elif dtype != a.dtype:
        raise TypeError(f"Expected dtype '{dtype}', got {a.dtype}")
    return a.dtype


def _raise_exception_on_failure(result=1):
    if result:
        size = _c_size_t(0)
        _lib.gn_error_string_size(_ctypes.byref(size))
        buf = _ctypes.create_string_buffer(size.value)
        result = _lib.gn_error_string(buf, len(buf))
        if result:
            msg2 = (
                "An error was reported, but the error message could not be retrieved."
            )
        else:
            msg2 = buf.value.decode("utf-8")
            if not msg2:
                msg2 = "An error was reported, but the error message was empty."
        raise Exception(f"gnnalysis error:\n\t{msg2}")


def _enum_value(enumeration, enumerator):
    value = _c_int(0)
    enumeration = bytes(enumeration, "utf-8")
    enumerator = bytes(enumerator, "utf-8")
    result = _lib.gn_enum_value(_ctypes.byref(value), enumeration, enumerator)
    _raise_exception_on_failure(result)
    return value.value


def _get_analysis_containers(analysis_type):
    size = _c_size_t(0)
    result = _lib.gn_analysis_results_size(_ctypes.byref(size), analysis_type)
    _raise_exception_on_failure(result)
    size = size.value
    key_sizes = (_c_size_t * size)()
    result = _lib.gn_analysis_results_key_sizes(key_sizes, size, analysis_type)
    _raise_exception_on_failure(result)
    keys = (_c_char_p * size)()
    values = (_c_double * size)()
    for i in range(size):
        keys[i] = _ctypes.cast(
            _ctypes.create_string_buffer(int(key_sizes[i])), _c_char_p
        )
    return keys, values


def _get_key_value_arrays(d):
    if not isinstance(d, dict):
        raise TypeError(f"Expected dict, got {type(d).__name__}")
    size = len(d)
    keys, values = list(d.keys()), list(d.values())
    ckeys = (_c_char_p * size)()
    cvalues = (_c_double * size)()
    for i in range(size):
        ckeys[i] = _ctypes.cast(
            _ctypes.create_string_buffer(bytes(str(keys[i]), "utf-8")), _c_char_p
        )
        cvalues[i] = values[i]
    return ckeys, cvalues


def _make_results_dict(keys, values):
    results = dict()
    for i in range(len(keys)):
        key = keys[i].decode("utf-8")
        results[key] = values[i]
    return results


"""
Enumerations
"""


class _AnalysisType(_IntEnum):  # Intentionally private
    DNL = _enum_value("AnalysisType", "DNL")
    FOURIER = _enum_value("AnalysisType", "Fourier")
    HISTOGRAM = _enum_value("AnalysisType", "Histogram")
    INL = _enum_value("AnalysisType", "INL")
    WAVEFORM = _enum_value("AnalysisType", "Waveform")


class CodeFormat(_IntEnum):
    """Specifies the binary encoding format of ADC output codes.

    Attributes:
        ``OFFSET_BINARY`` : Offset Binary (unsigned, zero at midscale)

        ``TWOS_COMPLEMENT`` : Two's Complement (signed, zero at midscale)
    """

    OFFSET_BINARY = _enum_value("CodeFormat", "OffsetBinary")
    TWOS_COMPLEMENT = _enum_value("CodeFormat", "TwosComplement")


class DnlSignal(_IntEnum):
    """Specifies the type of stimulus signal used when computing DNL.

    The computation method differs between ramp and tone (sinusoidal) stimulus
    because each produces a different expected probability density function.

    Attributes:
        ``RAMP`` : Ramp (linear) stimulus signal

        ``TONE`` : Tone (sinusoidal) stimulus signal
    """

    RAMP = _enum_value("DnlSignal", "Ramp")
    TONE = _enum_value("DnlSignal", "Tone")


class FaCompTag(_IntEnum):
    """Enumerates Fourier analysis component tags

    Attributes:
        ``DC`` : DC component (always Bin 0)

        ``SIGNAL`` : Signal component

        ``HD`` : Harmonic distortion

        ``IMD`` : Intermodulation distortion

        ``ILOS`` : Interleaving offset component

        ``ILGT`` : Interleaving gain/timing/BW component

        ``CLK`` : Clock component

        ``USERDIST`` : User-designated distortion

        ``NOISE`` : Noise component (e.g. WorstOther)
    """

    DC = _enum_value("FACompTag", "DC")
    SIGNAL = _enum_value("FACompTag", "Signal")
    HD = _enum_value("FACompTag", "HD")
    IMD = _enum_value("FACompTag", "IMD")
    ILOS = _enum_value("FACompTag", "ILOS")
    ILGT = _enum_value("FACompTag", "ILGT")
    CLK = _enum_value("FACompTag", "CLK")
    USERDIST = _enum_value("FACompTag", "UserDist")
    NOISE = _enum_value("FACompTag", "Noise")


class FaSsb(_IntEnum):
    """Enumerates the component categories for which the number of single side bins (SSB) can be set

    Attributes:
        ``DEFAULT`` : Default SSB (applies to auto-generated components)

        ``DC`` : SSB for DC component

        ``SIGNAL`` : SSB for Signal components

        ``WO`` : SSB for WorstOther components
    """

    DEFAULT = _enum_value("FASsb", "Default")
    DC = _enum_value("FASsb", "DC")
    SIGNAL = _enum_value("FASsb", "Signal")
    WO = _enum_value("FASsb", "WO")


class FreqAxisFormat(_IntEnum):
    """Enumerates frequency axis formats

    Attributes:
        ``BINS`` : FFT bin indices (0, 1, 2, ...)

        ``FREQ`` : Frequency in Hz

        ``NORM`` : Normalized frequency (cycles per sample)
    """

    BINS = _enum_value("FreqAxisFormat", "Bins")
    FREQ = _enum_value("FreqAxisFormat", "Freq")
    NORM = _enum_value("FreqAxisFormat", "Norm")


class FreqAxisType(_IntEnum):
    """Enumerates frequency axis types

    Attributes:
        ``DC_CENTER`` : DC centered, e.g. [-fs/2, fs/2) (complex FFT only)

        ``DC_LEFT`` : DC on left, e.g. [0, fs) (complex FFT only)

        ``REAL`` : Real axis, e.g. [0, fs/2] (real FFT only)
    """

    DC_CENTER = _enum_value("FreqAxisType", "DcCenter")
    DC_LEFT = _enum_value("FreqAxisType", "DcLeft")
    REAL = _enum_value("FreqAxisType", "Real")


class InlLineFit(_IntEnum):
    """Specifies the line-fitting method used when computing INL.

    Attributes:
        ``BEST_FIT`` : Least-squares best fit line

        ``END_FIT`` : Line through the first and last points

        ``NO_FIT`` : No line fitting; raw cumulative sum of DNL
    """

    BEST_FIT = _enum_value("InlLineFit", "BestFit")
    END_FIT = _enum_value("InlLineFit", "EndFit")
    NO_FIT = _enum_value("InlLineFit", "NoFit")


class RfftScale(_IntEnum):
    """Specifies the dBFS scaling convention for real FFT output.

    Attributes:
        ``DBFS_DC`` : dBFS relative to a full-scale DC signal; a full-scale sinusoid measures -3.01 dBFS

        ``DBFS_SIN`` : dBFS relative to a full-scale sinusoid; a full-scale sinusoid measures 0 dBFS

        ``NATIVE`` : Native scaling; a full-scale sinusoid measures -6.02 dBFS
    """

    DBFS_DC = _enum_value("RfftScale", "DbfsDc")
    DBFS_SIN = _enum_value("RfftScale", "DbfsSin")
    NATIVE = _enum_value("RfftScale", "Native")


class Window(_IntEnum):
    """Specifies the window function applied before FFT computation.

    Windowing reduces spectral leakage for non-coherently sampled signals.

    Attributes:
        ``BLACKMAN_HARRIS`` : 4-term Blackman-Harris window, excellent sidelobe suppression

        ``HANN`` : Hann (Hanning) window, good general-purpose choice

        ``NO_WINDOW`` : Rectangular window (no windowing), use only with coherent sampling
    """

    BLACKMAN_HARRIS = _enum_value("Window", "BlackmanHarris")
    HANN = _enum_value("Window", "Hann")
    NO_WINDOW = _enum_value("Window", "NoWindow")


del _IntEnum


"""
API Utilities
"""


def _version_string():
    size = _c_size_t(0)
    _lib.gn_version_string_size(_ctypes.byref(size))
    buf = _ctypes.create_string_buffer(size.value)
    result = _lib.gn_version_string(buf, len(buf))
    _raise_exception_on_failure(result)
    return buf.value.decode("utf-8")


"""
Array Operations
"""

_lib.gn_abs.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t]
_lib.gn_angle.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t]
_lib.gn_db.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t]
_lib.gn_db10.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t]
_lib.gn_db20.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t]
_lib.gn_norm.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t]


def abs(a):
    """Compute the magnitude of each element in a complex array.

    Computes |z| for each complex number z in the input array.

    Parameters
    ----------
    a : ndarray of dtype 'complex128'
        Input array of complex values.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of absolute values (magnitudes).

    """
    _check_ndarray(a, "complex128")
    out = _np.empty(a.size, dtype="float64")
    af64 = a.view("float64")
    result = _lib.gn_abs(out, out.size, af64, af64.size)
    _raise_exception_on_failure(result)
    return out


def angle(a):
    """Compute the phase angle (argument) of each element in a complex array.

    Returns the angle in radians for each complex number in the input array.

    Parameters
    ----------
    a : ndarray of dtype 'complex128'
        Input array of complex values.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of phase angles in radians.

    """
    _check_ndarray(a, "complex128")
    out = _np.empty(a.size, dtype="float64")
    af64 = a.view("float64")
    result = _lib.gn_angle(out, out.size, af64, af64.size)
    _raise_exception_on_failure(result)
    return out


def db(a):
    """Convert complex values to decibels.

    Computes 10*log10(|z|^2) = 20*log10(|z|) for each complex number z.

    Parameters
    ----------
    a : ndarray of dtype 'complex128'
        Input array of complex values.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of values in decibels.

    """
    _check_ndarray(a, "complex128")
    out = _np.empty(a.size, dtype="float64")
    af64 = a.view("float64")
    result = _lib.gn_db(out, out.size, af64, af64.size)
    _raise_exception_on_failure(result)
    return out


def db10(a):
    """Convert real values to decibels using 10*log10(x).

    Useful for power quantities where doubling the value adds 3 dB.

    Parameters
    ----------
    a : ndarray of dtype 'float64'
        Input array of real (power) values.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of values in decibels.

    """
    _check_ndarray(a, "float64")
    out = _np.empty(a.size, dtype="float64")
    result = _lib.gn_db10(out, out.size, a, a.size)
    _raise_exception_on_failure(result)
    return out


def db20(a):
    """Convert real values to decibels using 20*log10(x).

    Useful for amplitude or voltage quantities where doubling the value adds 6 dB.

    Parameters
    ----------
    a : ndarray of dtype 'float64'
        Input array of real (amplitude) values.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of values in decibels.

    """
    _check_ndarray(a, "float64")
    out = _np.empty(a.size, dtype="float64")
    result = _lib.gn_db20(out, out.size, a, a.size)
    _raise_exception_on_failure(result)
    return out


def norm(a):
    """Compute the squared magnitude of each element in a complex array.

    Computes re^2 + im^2 for each complex number, equivalent to |z|^2.

    Parameters
    ----------
    a : ndarray of dtype 'complex128'
        Input array of complex values.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of squared magnitudes.

    """
    _check_ndarray(a, "complex128")
    out = _np.empty(a.size, dtype="float64")
    af64 = a.view("float64")
    result = _lib.gn_norm(out, out.size, af64, af64.size)
    _raise_exception_on_failure(result)
    return out


"""
Code Density
"""

_lib.gn_code_axis.argtypes = [_ndptr_f64_1d, _c_size_t, _c_int, _c_int]
_lib.gn_code_axisx.argtypes = [_ndptr_f64_1d, _c_size_t, _c_int64, _c_int64]
_lib.gn_code_density_size.argtypes = [_c_size_t_p, _c_int, _c_int]
_lib.gn_code_densityx_size.argtypes = [_c_size_t_p, _c_int64, _c_int64]
_lib.gn_dnl.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_u64_1d, _c_size_t, _c_int]
_lib.gn_dnl_analysis.argtypes = [
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
]
_lib.gn_hist16.argtypes = [
    _ndptr_u64_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _c_int,
    _c_int,
    _c_bool,
]
_lib.gn_hist32.argtypes = [
    _ndptr_u64_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _c_int,
    _c_int,
    _c_bool,
]
_lib.gn_hist64.argtypes = [
    _ndptr_u64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _c_int,
    _c_int,
    _c_bool,
]
_lib.gn_histx16.argtypes = [
    _ndptr_u64_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _c_int64,
    _c_int64,
    _c_bool,
]
_lib.gn_histx32.argtypes = [
    _ndptr_u64_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _c_int64,
    _c_int64,
    _c_bool,
]
_lib.gn_histx64.argtypes = [
    _ndptr_u64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _c_int64,
    _c_int64,
    _c_bool,
]
_lib.gn_hist_analysis.argtypes = [
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _ndptr_u64_1d,
    _c_size_t,
]
_lib.gn_inl.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t, _c_int]
_lib.gn_inl_analysis.argtypes = [
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
]


def code_axis(n, fmt=CodeFormat.TWOS_COMPLEMENT):
    """Generate an array of code values for an ADC with n-bit resolution.

    Used as the x-axis when plotting DNL or INL. For example, a 3-bit two's
    complement ADC produces codes [-4, -3, -2, -1, 0, 1, 2, 3].

    Parameters
    ----------
    n : int
        ADC resolution in bits.
    fmt : CodeFormat
        Binary code format (default: TWOS_COMPLEMENT).

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of code values.

    """
    size = _c_size_t(0)
    result = _lib.gn_code_density_size(_ctypes.byref(size), n, fmt)
    _raise_exception_on_failure(result)
    out = _np.empty(size.value, dtype="float64")
    result = _lib.gn_code_axis(out, out.size, n, fmt)
    _raise_exception_on_failure(result)
    return out


def code_axisx(min_code, max_code):
    """Generate an array of code values for an explicit code range.

    Used when the code range does not follow a standard n-bit format.

    Parameters
    ----------
    min_code : int
        Minimum code value.
    max_code : int
        Maximum code value.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of code values from min_code to max_code.

    """
    size = _c_size_t(0)
    result = _lib.gn_code_densityx_size(_ctypes.byref(size), min_code, max_code)
    _raise_exception_on_failure(result)
    out = _np.empty(size.value, dtype="float64")
    result = _lib.gn_code_axisx(out, out.size, min_code, max_code)
    _raise_exception_on_failure(result)
    return out


def dnl(a, signal_type=DnlSignal.TONE):
    """Compute Differential Nonlinearity (DNL) from a histogram array.

    DNL measures the deviation of each code bin width from the ideal 1-LSB step
    size. A perfect ADC has DNL of 0 for all codes. Missing codes have DNL = -1.

    Parameters
    ----------
    a : ndarray of dtype 'uint64'
        Histogram data (from ``hist()`` or ``histx()``).
    signal_type : DnlSignal
        Type of stimulus signal used to generate the histogram (RAMP or TONE).

    Returns
    -------
    out : ndarray of dtype 'float64'
        DNL values in LSBs.
    """
    _check_ndarray(a, "uint64")
    out = _np.empty(a.size, dtype="float64")
    result = _lib.gn_dnl(out, out.size, a, a.size, signal_type)
    _raise_exception_on_failure(result)
    return out


def dnl_analysis(a):
    """Compute summary statistics of DNL data.

    Computes min, max, average, RMS, and non-missing code range from DNL data.

    Parameters
    ----------
    a : ndarray of dtype 'float64'
        DNL data (from ``dnl()``).

    Returns
    -------
    results : dict
        Every Key:Value pair in the dictionary is str:float.

        ====================================================================================
          Key             |  Description
        ====================================================================================
          min             |  Minumum value
          max             |  Maximum value
          avg             |  Average value
          rms             |  RMS value
          min_index       |  Index of first occurence of minimum value
          max_index       |  Index of first occurence of maximum value
          first_nm_index  |  Index of first non-missing code
          last_nm_index   |  Index of last non-missing code
          nm_range        |  Non-missing code range (1 + (last_nm_index - first_nm_index))
        ====================================================================================

    """
    _check_ndarray(a, "float64")
    keys, values = _get_analysis_containers(_AnalysisType.DNL)
    result = _lib.gn_dnl_analysis(keys, len(keys), values, len(values), a, a.size)
    _raise_exception_on_failure(result)
    results = _make_results_dict(keys, values)
    return results


def hist(a, n, fmt=CodeFormat.TWOS_COMPLEMENT):
    """Compute a code density histogram of a quantized waveform.

    Each bin counts the number of occurrences of each code value. Used as
    input to ``dnl()`` for linearity analysis.

    Parameters
    ----------
    a : ndarray of dtype 'int16', 'int32', or 'int64'
        Quantized waveform.
    n : int
        ADC resolution in bits.
    fmt : CodeFormat
        Binary code format (default: TWOS_COMPLEMENT).

    Returns
    -------
    out : ndarray of dtype 'uint64'
        Array of bin counts, one per code value.

    """
    dtype = _check_ndarray(a, ["int16", "int32", "int64"])
    size = _c_size_t(0)
    result = _lib.gn_code_density_size(_ctypes.byref(size), n, fmt)
    _raise_exception_on_failure(result)
    out = _np.zeros(size.value, dtype="uint64")
    if "int16" == dtype:
        result = _lib.gn_hist16(out, out.size, a, a.size, n, fmt, False)
    elif "int32" == dtype:
        result = _lib.gn_hist32(out, out.size, a, a.size, n, fmt, False)
    else:
        result = _lib.gn_hist64(out, out.size, a, a.size, n, fmt, False)
    _raise_exception_on_failure(result)
    return out


def histx(a, min_code, max_code):
    """Compute a code density histogram using explicit code bounds.

    Each bin counts occurrences of codes from min_code to max_code. Use this
    instead of ``hist()`` when the code range does not follow a standard n-bit format.

    Parameters
    ----------
    a : ndarray of dtype 'int16', 'int32', or 'int64'
        Quantized waveform.
    min_code : int
        Minimum code value.
    max_code : int
        Maximum code value.

    Returns
    -------
    out : ndarray of dtype 'uint64'
        Array of bin counts, one per code value.

    """
    dtype = _check_ndarray(a, ["int16", "int32", "int64"])
    size = _c_size_t(0)
    result = _lib.gn_code_densityx_size(_ctypes.byref(size), min_code, max_code)
    _raise_exception_on_failure(result)
    out = _np.zeros(size.value, dtype="uint64")
    if "int16" == dtype:
        result = _lib.gn_histx16(out, out.size, a, a.size, min, max, False)
    elif "int32" == dtype:
        result = _lib.gn_histx32(out, out.size, a, a.size, min, max, False)
    else:
        result = _lib.gn_histx64(out, out.size, a, a.size, min, max, False)
    _raise_exception_on_failure(result)
    return out


def hist_analysis(a):
    """Compute summary statistics of histogram data.

    Computes sum, first/last non-zero bin indices, and non-zero range.

    Parameters
    ----------
    a : ndarray of dtype 'uint64'
        Histogram data (from ``hist()`` or ``histx()``).

    Returns
    -------
    results : dict
        Every Key:Value pair in the dictionary is str:float.

        ================================================================================
          Key             |  Description
        ================================================================================
          sum             |  Sum of all histogram bins
          first_nz_index  |  First non-zero bin
          last_nz_index   |  Last non-zero bin
          nz_range        |  Non-zero bin range (1 + (last_nz_index - first_nz_index))
        ================================================================================

    """
    _check_ndarray(a, "uint64")
    keys, values = _get_analysis_containers(_AnalysisType.HISTOGRAM)
    result = _lib.gn_hist_analysis(keys, len(keys), values, len(values), a, a.size)
    _raise_exception_on_failure(result)
    results = _make_results_dict(keys, values)
    return results


def inl(a, fit=InlLineFit.BEST_FIT):
    """Compute Integral Nonlinearity (INL) from DNL data by cumulative summation.

    INL measures the deviation of the ADC transfer function from an ideal
    straight line. The fit parameter controls whether a best-fit or endpoint
    line is subtracted.

    Parameters
    ----------
    a : ndarray of dtype 'float64'
        DNL data (from ``dnl()``).
    fit : InlLineFit
        Line fitting method (default: BEST_FIT).

    Returns
    -------
    out : ndarray of dtype 'float64'
        INL values in LSBs.
    """
    _check_ndarray(a, "float64")
    out = _np.empty(a.size, dtype="float64")
    result = _lib.gn_inl(out, out.size, a, a.size, fit)
    _raise_exception_on_failure(result)
    return out


def inl_analysis(a):
    """Compute summary statistics of INL data.

    Computes min, max, and their indices from INL data.

    Parameters
    ----------
    a : ndarray of dtype 'float64'
        INL data (from ``inl()``).

    Returns
    -------
    results : dict
        Every Key:Value pair in the dictionary is str:float.

        ===========================================================
          Key        |  Description
        ===========================================================
          min        |  Minumum value
          max        |  Maximum value
          min_index  |  Index of first occurence of minimum value
          max_index  |  Index of first occurence of maximum value
        ===========================================================

    """
    _check_ndarray(a, "float64")
    keys, values = _get_analysis_containers(_AnalysisType.INL)
    result = _lib.gn_inl_analysis(keys, len(keys), values, len(values), a, a.size)
    _raise_exception_on_failure(result)
    results = _make_results_dict(keys, values)
    return results


"""
Fourier Analysis
"""

_lib.gn_fft_analysis.argtypes = [
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _c_char_p,
    _ndptr_f64_1d,
    _c_size_t,
    _c_size_t,
    _c_int,
]
_lib.gn_fft_analysis_results_key_sizes.argtypes = [
    _c_size_t_p,
    _c_size_t,
    _c_char_p,
    _c_size_t,
    _c_size_t,
]
_lib.gn_fft_analysis_results_size.argtypes = [
    _c_size_t_p,
    _c_char_p,
    _c_size_t,
    _c_size_t,
]


def fft_analysis(test_key, a, nfft, axis_type=FreqAxisType.DC_LEFT):
    """Returns all Fourier analysis results

    Args:
        ``test_key`` (``string``) : Key value to the Fourier Analysis object created (through gn_fa_create)

        ``a`` (``ndarray``) : FFT data of type 'complex128' or 'float64'

        ``nfft`` (``int``) : FFT size

        axis_type (``FreqAxisType``) : Frequency axis type

    Returns:
        ``results`` (``dict``) : Dictionary containing all Fourier analysis results

    Notes:
        Every Key:Value pair in the dictionary is ``str``:``float``.

        The dictionary contains the following keys:
            ``signaltype`` : Signal type: 0=Real, 1=Complex

            ``nfft`` : FFT size

            ``datasize`` : Data size

            ``fbin`` : Frequency bin size (Hz)

            ``fdata`` : Data rate (S/s)

            ``fsample`` : Sample rate (S/s)

            ``fshift`` : Shift frequency (Hz)

            ``fsnr`` : Full-scale-to-noise ratio (a.k.a. "SNRFS") (dB)

            ``snr`` : Signal-to-noise ratio (dB)

            ``sinad`` : Signal-to-noise-and-distortion ratio (dB)

            ``sfdr`` : Spurious-free dynamic range (dB)

            ``abn`` : Average bin noise (dBFS)

            ``nsd`` : Noise spectral density (dBFS/Hz)

            ``carrierindex`` : Order index of the Carrier tone

            ``maxspurindex`` : Order index of the MaxSpur tone

            ``ab_width`` : Analysis band width (Hz)

            ``ab_i1`` : Analysis band first index

            ``ab_i2`` : Analysis band last index

            ``{PREFIX}_nbins`` : Number of bins associated with PREFIX

            ``{PREFIX}_rss`` : Root-sum-square associated with PREFIX

            ``{TONEKEY}:ffinal`` : Tone final frequency (Hz)

            ``{TONEKEY}:freq`` : Tone frequency (Hz)

            ``{TONEKEY}:fwavg`` : Tone weighted-average frequency (Hz)

            ``{TONEKEY}:i1`` : Tone first index

            ``{TONEKEY}:i2`` : Tone last index

            ``{TONEKEY}:inband`` : 1: tone is in-band; 0: tone is out-of-band

            ``{TONEKEY}:mag`` : Tone magnitude

            ``{TONEKEY}:mag_dbc`` : Tone magnitude relative to carrier (dBc)

            ``{TONEKEY}:mag_dbfs`` : Tone magnitude relative to full-scale (dBFS)

            ``{TONEKEY}:nbins`` : Tone number of bins

            ``{TONEKEY}:orderindex`` : Tone order index

            ``{TONEKEY}:phase`` : Tone phase (rad)

            ``{TONEKEY}:phase_c`` : Tone phase relative to carrier (rad)

            ``{TONEKEY}:tag`` : Tone tag
    """
    test_key = bytes(test_key, "utf-8")
    dtype = _check_ndarray(a, ["complex128", "float64"])
    af64 = a.view("float64") if "complex128" == dtype else a
    size = _c_size_t(0)
    result = _lib.gn_fft_analysis_results_size(
        _ctypes.byref(size), test_key, af64.size, nfft
    )
    _raise_exception_on_failure(result)
    size = size.value
    key_sizes = (_c_size_t * size)()
    result = _lib.gn_fft_analysis_results_key_sizes(
        key_sizes, size, test_key, af64.size, nfft
    )
    _raise_exception_on_failure(result)
    keys = (_c_char_p * size)()
    values = (_c_double * size)()
    for i in range(size):
        keys[i] = _ctypes.cast(
            _ctypes.create_string_buffer(int(key_sizes[i])), _c_char_p
        )
    result = _lib.gn_fft_analysis(
        keys, size, values, size, test_key, af64, af64.size, nfft, axis_type
    )
    _raise_exception_on_failure(result)
    results = _make_results_dict(keys, values)
    return results


"""
Fourier Analysis Configuration
"""

_lib.gn_fa_analysis_band.argtypes = [_c_char_p, _c_double, _c_double]
_lib.gn_fa_analysis_band_e.argtypes = [_c_char_p, _c_char_p, _c_char_p]
_lib.gn_fa_clk.argtypes = [_c_char_p, _ndptr_i32_1d, _c_size_t, _c_bool]
_lib.gn_fa_create.argtypes = [_c_char_p]
_lib.gn_fa_fdata.argtypes = [_c_char_p, _c_double]
_lib.gn_fa_fdata_e.argtypes = [_c_char_p, _c_char_p]
_lib.gn_fa_fixed_tone.argtypes = [_c_char_p, _c_char_p, _c_int, _c_double, _c_int]
_lib.gn_fa_fixed_tone_e.argtypes = [_c_char_p, _c_char_p, _c_int, _c_char_p, _c_int]
_lib.gn_fa_fsample.argtypes = [_c_char_p, _c_double]
_lib.gn_fa_fsample_e.argtypes = [_c_char_p, _c_char_p]
_lib.gn_fa_fshift.argtypes = [_c_char_p, _c_double]
_lib.gn_fa_fshift_e.argtypes = [_c_char_p, _c_char_p]
_lib.gn_fa_ilv.argtypes = [_c_char_p, _ndptr_i32_1d, _c_size_t, _c_bool]
_lib.gn_fa_load.argtypes = [_c_char_p, _c_size_t, _c_char_p, _c_char_p]
_lib.gn_fa_max_tone.argtypes = [_c_char_p, _c_char_p, _c_int, _c_int]
_lib.gn_fa_preview.argtypes = [_c_char_p, _c_size_t, _c_char_p, _c_bool]
_lib.gn_fa_remove_comp.argtypes = [_c_char_p, _c_char_p]
_lib.gn_fa_var.argtypes = [_c_char_p, _c_char_p, _c_double]


def fa_analysis_band(test_key, center, width):
    """Set the analysis band, limiting metric computations to a specific frequency range.

    Only spectral content within [center - width/2, center + width/2] is included
    in SNR, SINAD, and other metrics.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``center`` (``float`` or ``str``) : Analysis band center in Hz, or an expression string referencing variables

        ``width`` (``float`` or ``str``) : Analysis band width in Hz, or an expression string referencing variables
    """
    test_key = bytes(test_key, "utf-8")
    if isinstance(center, str) and isinstance(width, str):
        center = bytes(center, "utf-8")
        width = bytes(width, "utf-8")
        result = _lib.gn_fa_analysis_band_e(test_key, center, width)
    else:
        result = _lib.gn_fa_analysis_band(test_key, center, width)
    _raise_exception_on_failure(result)


def fa_clk(test_key, x, as_noise=False):
    """Configure clock sub-harmonic divisors for identifying clock spurs.

    For each divisor d in x, spurs at fs/d and its harmonics are identified
    and tagged as CLK components. If as_noise is True, these are classified
    as noise rather than distortion.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``x`` (``list`` of ``int``) : Clock divisors

        ``as_noise`` (``bool``) : If True, CLK components are classified as noise instead of distortion
    """
    test_key = bytes(test_key, "utf-8")
    x = _np.array(x, dtype="int32")
    result = _lib.gn_fa_clk(test_key, x, x.size, as_noise)
    _raise_exception_on_failure(result)


def fa_conv_offset(test_key, enable):
    """Enable or disable the converter offset component.

    When enabled, a tone at the converter offset frequency is identified in
    the spectrum.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``enable`` (``bool``) : If True, enable the converter offset component
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_conv_offset(test_key, enable)
    _raise_exception_on_failure(result)


def fa_create(test_key):
    """Create a new Fourier analysis configuration object.

    Registers the object in the global object manager under the given key.
    This must be called before any other ``fa_*`` configuration functions.

    Args:
        ``test_key`` (``str``) : Test key under which to register the object
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_create(test_key)
    _raise_exception_on_failure(result)


def fa_dc(test_key, as_dist):
    """Control whether the DC component (bin 0) is classified as distortion or noise.

    By default, DC is treated as noise. Setting as_dist=True includes DC power
    in distortion metrics like THD.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``as_dist`` (``bool``) : If True, classify DC as distortion; if False, classify as noise
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_dc(test_key, as_dist)
    _raise_exception_on_failure(result)


def fa_fdata(test_key, f):
    """Set the data rate (effective sample rate of the signal).

    In systems with digital downconversion, fdata may differ from fsample.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``f`` (``float`` or ``str``) : Data rate in S/s, or an expression string referencing variables
    """
    test_key = bytes(test_key, "utf-8")
    if isinstance(f, str):
        f = bytes(f, "utf-8")
        result = _lib.gn_fa_fdata_e(test_key, f)
    else:
        result = _lib.gn_fa_fdata(test_key, f)
    _raise_exception_on_failure(result)


def fa_fixed_tone(test_key, comp_key, tag, freq, ssb=-1):
    """Add a tone component at a specific frequency.

    The tone is identified by comp_key and classified according to tag
    (SIGNAL, HD, IMD, etc.).

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``comp_key`` (``str``) : Unique identifier for this component

        ``tag`` (``FaCompTag``) : Component classification tag

        ``freq`` (``float`` or ``str``) : Tone frequency in Hz, or an expression string referencing variables

        ``ssb`` (``int``) : Number of single-side bins (-1 uses the group default)
    """
    test_key = bytes(test_key, "utf-8")
    comp_key = bytes(comp_key, "utf-8")
    if isinstance(freq, str):
        freq = bytes(freq, "utf-8")
        result = _lib.gn_fa_fixed_tone_e(test_key, comp_key, tag, freq, ssb)
    else:
        result = _lib.gn_fa_fixed_tone(test_key, comp_key, tag, freq, ssb)
    _raise_exception_on_failure(result)


def fa_fsample(test_key, f):
    """Set the sample rate of the ADC.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``f`` (``float`` or ``str``) : Sample rate in S/s, or an expression string referencing variables
    """
    test_key = bytes(test_key, "utf-8")
    if isinstance(f, str):
        f = bytes(f, "utf-8")
        result = _lib.gn_fa_fsample_e(test_key, f)
    else:
        result = _lib.gn_fa_fsample(test_key, f)
    _raise_exception_on_failure(result)


def fa_fshift(test_key, f):
    """Set the cumulative frequency shift applied to the signal after sampling.

    Used to account for digital downconversion or other frequency translation
    applied after the ADC.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``f`` (``float`` or ``str``) : Shift frequency in Hz, or an expression string referencing variables
    """
    test_key = bytes(test_key, "utf-8")
    if isinstance(f, str):
        f = bytes(f, "utf-8")
        result = _lib.gn_fa_fshift_e(test_key, f)
    else:
        result = _lib.gn_fa_fshift(test_key, f)
    _raise_exception_on_failure(result)


def fa_fund_images(test_key, enable):
    """Enable or disable fundamental image components in complex FFT analysis.

    When enabled, the image of each signal tone is identified and classified.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``enable`` (``bool``) : If True, enable fundamental image components
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_fund_images(test_key, enable)
    _raise_exception_on_failure(result)


def fa_hd(test_key, n):
    """Set the maximum harmonic distortion order.

    Harmonics 2 through n of each signal tone are automatically generated
    and tracked. For example, n=5 tracks HD2 through HD5.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``n`` (``int``) : Maximum harmonic order
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_hd(test_key, n)
    _raise_exception_on_failure(result)


def fa_ilv(test_key, x, as_noise=False):
    """Configure interleaving factors for identifying interleaving spurs.

    For each factor f in x, offset and gain/timing spurs at fs/f offsets from
    signal tones are tracked. If as_noise is True, these are classified as
    noise rather than distortion.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``x`` (``list`` of ``int``) : Interleaving factors

        ``as_noise`` (``bool``) : If True, interleaving spurs are classified as noise instead of distortion
    """

    test_key = bytes(test_key, "utf-8")
    x = _np.array(x, dtype="int32")
    result = _lib.gn_fa_ilv(test_key, x, x.size, as_noise)
    _raise_exception_on_failure(result)


def fa_imd(test_key, n):
    """Set the maximum intermodulation distortion order.

    When multiple signal tones are present, IMD products up to order n are
    automatically generated and tracked.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``n`` (``int``) : Maximum intermodulation distortion order
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_imd(test_key, n)
    _raise_exception_on_failure(result)


def fa_load(filename, test_key=""):
    """Load a Fourier analysis configuration from a JSON file.

    Registers the loaded object in the global object manager and returns
    the key under which it was registered.

    Args:
        ``filename`` (``str``) : Path to the JSON configuration file

        ``test_key`` (``str``) : Optional key to register the object under (if empty, uses key from file)

    Returns:
        ``str`` : The key under which the object was registered
    """
    filename = bytes(filename, "utf-8")
    test_key = bytes(test_key, "utf-8")
    size = _c_size_t(0)
    result = _lib.gn_fa_load_key_size(_ctypes.byref(size), filename, test_key)
    _raise_exception_on_failure(result)
    buf = _ctypes.create_string_buffer(size.value)
    result = _lib.gn_fa_load(buf, len(buf), filename, test_key)
    _raise_exception_on_failure(result)
    return buf.value.decode("utf-8")


def fa_max_tone(test_key, comp_key, tag, ssb=-1):
    """Add a tone component located at the maximum magnitude in the spectrum.

    The tone is placed at the largest spectral bin not already assigned to
    another component. Useful for finding the strongest signal when its exact
    frequency is unknown.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``comp_key`` (``str``) : Unique identifier for this component

        ``tag`` (``FaCompTag``) : Component classification tag

        ``ssb`` (``int``) : Number of single-side bins (-1 uses the group default)
    """
    test_key = bytes(test_key, "utf-8")
    comp_key = bytes(comp_key, "utf-8")
    result = _lib.gn_fa_max_tone(test_key, comp_key, tag, ssb)
    _raise_exception_on_failure(result)


def fa_preview(test_key, cplx=False):
    """Return a string showing the current analysis configuration.

    Displays all defined components and their frequencies. Useful for
    verifying configuration before running analysis.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``cplx`` (``bool``) : If True, include complex-specific components in the preview

    Returns:
        ``str`` : Formatted string representation of the analysis configuration
    """
    test_key = bytes(test_key, "utf-8")
    size = _c_size_t(0)
    result = _lib.gn_fa_preview_size(_ctypes.byref(size), test_key, cplx)
    _raise_exception_on_failure(result)
    buf = _ctypes.create_string_buffer(size.value)
    result = _lib.gn_fa_preview(buf, len(buf), test_key, cplx)
    _raise_exception_on_failure(result)
    return buf.value.decode("utf-8")


def fa_quad_errors(test_key, enable):
    """Enable or disable quadrature error components in complex FFT analysis.

    When enabled, image and gain/phase imbalance tones are identified and
    classified in the spectrum.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``enable`` (``bool``) : If True, enable quadrature error components
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_quad_errors(test_key, enable)
    _raise_exception_on_failure(result)


def fa_remove_comp(test_key, comp_key):
    """Remove a previously added component from the analysis configuration.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``comp_key`` (``str``) : Key of the component to remove
    """
    test_key = bytes(test_key, "utf-8")
    comp_key = bytes(comp_key, "utf-8")
    result = _lib.gn_fa_remove_comp(test_key, comp_key)
    _raise_exception_on_failure(result)


def fa_reset(test_key):
    """Reset the Fourier analysis object to its default configuration.

    Removes all user-defined components and variables.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_reset(test_key)
    _raise_exception_on_failure(result)


def fa_ssb(test_key, group, n):
    """Set the number of single-side bins (SSB) for a component group.

    Each tone occupies 2*n+1 FFT bins total (n bins on each side of the center
    bin plus the center bin itself). Windowed FFTs require SSB > 0 to capture
    spectral leakage.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``group`` (``FaSsb``) : Component group (DEFAULT, DC, SIGNAL, or WO)

        ``n`` (``int``) : Number of single-side bins
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_ssb(test_key, group, n)
    _raise_exception_on_failure(result)


def fa_ssb_dc(test_key, n):
    """Configure number of single-side bins for DC.

    .. deprecated::
        Use ``fa_ssb(test_key, FaSsb.DC, n)`` instead.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``n`` (``int``) : Number of single-side bins
    """
    print("fa_ssb_dc(test_key, n) is deprecated; use fa_ssb(test_key, FaSsb.DC, n)")
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_ssb(test_key, FaSsb.DC, n)
    _raise_exception_on_failure(result)


def fa_ssb_def(test_key, n):
    """Configure default number of single-side bins for auto-generated components.

    .. deprecated::
        Use ``fa_ssb(test_key, FaSsb.DEFAULT, n)`` instead.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``n`` (``int``) : Number of single-side bins
    """
    print(
        "fa_ssb_def(test_key, n) is deprecated; use fa_ssb(test_key, FaSsb.DEFAULT, n)"
    )
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_ssb(test_key, FaSsb.DEFAULT, n)
    _raise_exception_on_failure(result)


def fa_ssb_wo(test_key, n):
    """Configure number of single-side bins for WO components.

    .. deprecated::
        Use ``fa_ssb(test_key, FaSsb.WO, n)`` instead.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``n`` (``int``) : Number of single-side bins
    """
    print("fa_ssb_wo(test_key, n) is deprecated; use fa_ssb(test_key, FaSsb.WO, n)")
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_ssb(test_key, FaSsb.WO, n)
    _raise_exception_on_failure(result)


def fa_var(test_key, name, value):
    """Set the value of an expression variable.

    Variables can be referenced in frequency expressions used by ``fa_fdata``,
    ``fa_fsample``, ``fa_fshift``, and ``fa_fixed_tone``. For example, set
    variable "fs" to 1e9 and use "fs" in expressions.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``name`` (``str``) : Variable name

        ``value`` (``float``) : Variable value
    """
    test_key = bytes(test_key, "utf-8")
    name = bytes(name, "utf-8")
    result = _lib.gn_fa_var(test_key, name, value)
    _raise_exception_on_failure(result)


def fa_wo(test_key, n):
    """Set the number of worst-other (WO) tones to identify.

    WO tones are the n largest spectral components not classified as signal,
    harmonic, IMD, interleaving, or clock components. The largest WO tone
    determines SFDR.

    Args:
        ``test_key`` (``str``) : Test key (key to a Fourier Analysis object)

        ``n`` (``int``) : Number of worst-other tones to track
    """
    test_key = bytes(test_key, "utf-8")
    result = _lib.gn_fa_wo(test_key, n)
    _raise_exception_on_failure(result)


"""
Fourier Analysis Results
"""

_lib.gn_fa_result_string.argtypes = [
    _c_char_p,
    _c_size_t,
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _c_char_p,
]
_lib.gn_fa_result_string_size.argtypes = [
    _c_size_t_p,
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _c_char_p,
]


def _get_annot_boxes(axis_type, datasize, bottom, height, x1, x2, xscalar):
    boxes = []
    lbound = 0
    rbound = datasize - 1
    if FreqAxisType.DC_CENTER == axis_type:
        lbound -= datasize // 2
        rbound -= datasize // 2
        if rbound < x1:
            x1 -= datasize
        if rbound < x2:
            x2 -= datasize
    if x1 <= x2:  # 1 box, x1->x2
        left = (x1 - 0.5) * xscalar
        width = ((x2 - x1) + 1) * xscalar
        boxes.append((left, bottom, width, height))
    else:  # x2 < x1: 2 boxes, x1->rbound, lbound->x2
        if x1 < rbound:
            left = (x1 - 0.5) * xscalar
            width = rbound * xscalar - left
            boxes.append((left, bottom, width, height))
        if lbound < x2:
            left = lbound * xscalar
            width = (x2 + 0.5) * xscalar - left
            boxes.append((left, bottom, width, height))
    return boxes


def fa_annotations(
    result_dict, axis_type=FreqAxisType.DC_LEFT, axis_format=FreqAxisFormat.FREQ
):
    """Generate plot annotation data from Fourier analysis results.

    Returns a dictionary with annotation data for overlaying analysis results
    on FFT plots.

    Args:
        ``result_dict`` (``dict``) : Fourier analysis results dictionary (from ``fft_analysis()``)

        ``axis_type`` (``FreqAxisType``) : Frequency axis type (default: DC_LEFT)

        ``axis_format`` (``FreqAxisFormat``) : Frequency axis format (default: FREQ)

    Returns:
        ``dict`` : Dictionary with the following keys:

            ``labels`` : List of (x, y, str) tuples for tone labels with positions

            ``lines`` : List of (x1, y1, x2, y2) tuples for noise floor lines

            ``ab_boxes`` : List of (left, bottom, width, height) tuples for analysis band exclusion regions

            ``tone_boxes`` : List of (left, bottom, width, height) tuples for tone highlight regions
    """
    if not isinstance(result_dict, dict):
        raise TypeError(f"Expected dict, got {type(result_dict).__name__}")
    if _AnalysisType.FOURIER != result_dict["analysistype"]:
        raise ValueError("Expected Fourier analysis results")

    if 1.0 == result_dict["signaltype"]:  # complex
        if FreqAxisType.REAL == axis_type:
            axis_type = FreqAxisType.DC_LEFT
    else:
        axis_type = FreqAxisType.REAL

    datasize = result_dict["datasize"]
    fbin = result_dict["fbin"]
    if FreqAxisFormat.BINS == axis_format:
        xscalar = 1 / fbin
    elif FreqAxisFormat.NORM == axis_format:
        xscalar = 1 / result_dict["fdata"]
    else:
        xscalar = 1.0
    tone_keys = []
    for k in result_dict:
        if k.endswith(":ffinal"):
            tk = k.split(":")[0]
            tone_keys.append(tk)
    #
    # Labels = (x, y, str)
    #
    label_dict = dict()
    for tk in tone_keys:
        x = result_dict[tk + ":ffinal"] * xscalar
        y = result_dict[tk + ":mag_dbfs"]
        if x in label_dict:
            if label_dict[x][0] < y:
                label_dict[x][0] = y
            label_dict[x][1] += "\n" + tk
        else:
            label_dict[x] = [y, tk]
    labels = []
    for x in label_dict:
        labels.append((x, label_dict[x][0], label_dict[x][1]))
    #
    # Lines = (x1, y1, x2, y2)
    #
    lines = []
    if "abn" in result_dict:
        abn = result_dict["abn"]
        x = result_dict["fdata"] * xscalar
        lines.append((-x, abn, x, abn))
    #
    # Analysis Band Boxes = (xleft, ybottom, width, height)
    #
    ab_boxes = []
    if result_dict["ab_nbins"] < datasize:
        bottom = -300
        height = 600
        x1 = result_dict["ab_i2"] + 1  # "invert" the analysis band
        x2 = result_dict["ab_i1"] - 1  # to draw *excluded* spectrum
        ab_boxes += _get_annot_boxes(
            axis_type, datasize, bottom, height, x1, x2, fbin * xscalar
        )
    #
    # Tone Boxes = (xleft, ybottom, width, height)
    #
    tone_boxes = []
    for tk in tone_keys:
        bottom = -300
        height = result_dict[tk + ":mag_dbfs"] - bottom
        x1 = result_dict[tk + ":i1"]
        x2 = result_dict[tk + ":i2"]
        tone_boxes += _get_annot_boxes(
            axis_type, datasize, bottom, height, x1, x2, fbin * xscalar
        )

    annots = {
        "labels": labels,
        "lines": lines,
        "ab_boxes": ab_boxes,
        "tone_boxes": tone_boxes,
    }
    return annots


def fa_result_string(result_dict, result_key):
    """Format a single Fourier analysis result as a human-readable string.

    Looks up result_key in result_dict and returns a formatted string with
    appropriate units and precision.

    Args:
        ``result_dict`` (``dict``) : Fourier analysis results dictionary (from ``fft_analysis()``)

        ``result_key`` (``str``) : Key of the result to format

    Returns:
        ``str`` : Formatted result string with units
    """
    ckeys, cvalues = _get_key_value_arrays(result_dict)
    result_key = bytes(result_key, "utf-8")
    size = _c_size_t(0)
    result = _lib.gn_fa_result_string_size(
        _ctypes.byref(size), ckeys, len(ckeys), cvalues, len(cvalues), result_key
    )
    _raise_exception_on_failure(result)
    size = size.value
    buf = _ctypes.create_string_buffer(size)
    result = _lib.gn_fa_result_string(
        buf, size, ckeys, len(ckeys), cvalues, len(cvalues), result_key
    )
    _raise_exception_on_failure(result)
    return buf.value.decode("utf-8")


"""
Fourier Transforms
"""

_lib.gn_fft.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _c_size_t,
    _c_size_t,
    _c_int,
]
_lib.gn_fft16.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _c_int,
    _c_size_t,
    _c_size_t,
    _c_int,
    _c_int,
]
_lib.gn_fft32.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _c_int,
    _c_size_t,
    _c_size_t,
    _c_int,
    _c_int,
]
_lib.gn_fft64.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _c_int,
    _c_size_t,
    _c_size_t,
    _c_int,
    _c_int,
]
_lib.gn_fft_size.argtypes = [_c_size_t_p, _c_size_t, _c_size_t, _c_size_t, _c_size_t]
_lib.gn_rfft.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _c_size_t,
    _c_size_t,
    _c_int,
    _c_int,
]
_lib.gn_rfft16.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _c_int,
    _c_size_t,
    _c_size_t,
    _c_int,
    _c_int,
    _c_int,
]
_lib.gn_rfft32.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _c_int,
    _c_size_t,
    _c_size_t,
    _c_int,
    _c_int,
    _c_int,
]
_lib.gn_rfft64.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _c_int,
    _c_size_t,
    _c_size_t,
    _c_int,
    _c_int,
    _c_int,
]
_lib.gn_rfft_size.argtypes = [_c_size_t_p, _c_size_t, _c_size_t, _c_size_t]


def fft(a, *args):
    """
    Compute FFT

    Args:
        ``a`` (``ndarray``) : Input array of type ``complex128``, ``float64``, ``int16``, ``int32``, or ``int64``

        ``args`` (``list``) : Additional arguments
            1. If ``a`` is of type ``complex128`` or ``float64``, then compute the FFT of interleaved normalized samples with the following interpretation.

                ``navg`` (``int``) : FFT averaging number

                ``nfft`` (``int``) : FFT size

                ``window`` (``Window``): Window

                In this case, if ``a`` is not complex, then a is interpreted to contain interleaved I/Q samples with the following interpretation.

            2. If ``a`` is of type ``float64``, then compute the FFT of split normalized samples.

                ``q`` (``float64``) : Quadrature component

                ``navg`` (``int``) : FFT averaging number

                ``nfft`` (``int``) : FFT size

                ``window`` (``Window``): Window

                In this case, ``a`` is interpreted to be the In-phase component.

            3. If ``a`` is of type ``int16``, ``int32``, or ``int64``, then compute the FFT of interleaved quantized samples with the following interpretation.

                ``n`` (``int``) : Resolution (Bitwidth of a)

                ``navg`` (``int``) : FFT averaging number

                ``nfft`` (``int``) : FFT size

                ``window`` (``Window``): Window

                ``fmt`` (``CodeFormat``): Code format

                In this case, ``a`` is interpreted to contain interleaved quantized samples.

            4. If ``a`` is of type ``int16``, ``int32``, or ``int64``, then compute the FFT of split quantized samples with the following interpretation.

                ``q`` (``int16``, ``int32``, or ``int64``) : Quadrature component

                ``n`` (``int``) : Resolution (Bitwidth of ``a``)

                ``navg`` (``int``) : FFT averaging number

                ``nfft`` (``int``) : FFT size

                ``window`` (``Window``): Window

                ``fmt`` (``CodeFormat``): Code format

                In this case, ``a`` is interpreted to to be the In-phase component.

    Returns:
        ``out`` (``ndarray``) : FFT result of type ``float64`` with interleaved Re/Im components

    """
    dtype = _check_ndarray(a, ["complex128", "float64", "int16", "int32", "int64"])
    nargs = len(args)
    q_data = _np.empty(0, dtype=dtype)
    q_size = 0
    base_index = 0
    if dtype in ["complex128", "float64"]:  # normalized samples
        if "complex128" == dtype:
            i_data = a.view("float64")
            q_data.dtype = "float64"
        else:
            i_data = a
            if 0 < nargs and isinstance(args[0], _np.ndarray):  # arg[0] is Q data
                _check_ndarray(args[0], dtype)
                q_data = args[0]
                q_size = q_data.size
                base_index = 1
        i_size = i_data.size
        navg = 1 if nargs <= base_index else args[base_index]
        nfft = 0 if nargs <= base_index + 1 else args[base_index + 1]
        window = Window.NO_WINDOW if nargs <= base_index + 2 else args[base_index + 2]
        n = None
        fmt = None
    else:  # quantized samples
        i_data = a
        i_size = i_data.size
        if 0 < nargs and isinstance(args[0], _np.ndarray):  # arg[0] is Q data
            _check_ndarray(args[0], dtype)
            q_data = args[0]
            q_size = q_data.size
            base_index = 1
        if nargs <= base_index:
            raise Exception("Missing required parameter, n, code width")
        n = args[base_index]
        navg = 1 if nargs <= base_index + 1 else args[base_index + 1]
        nfft = 0 if nargs <= base_index + 2 else args[base_index + 2]
        window = Window.NO_WINDOW if nargs <= base_index + 3 else args[base_index + 3]
        fmt = (
            CodeFormat.TWOS_COMPLEMENT
            if nargs <= base_index + 4
            else args[base_index + 4]
        )
    out_size = _c_size_t(0)
    navg = max(0, navg)
    nfft = max(0, nfft)
    result = _lib.gn_fft_size(_ctypes.byref(out_size), i_size, q_size, navg, nfft)
    _raise_exception_on_failure(result)
    out = _np.empty(out_size.value // 2, dtype="complex128")
    outf64 = out.view("float64")
    if "int16" == dtype:
        result = _lib.gn_fft16(
            outf64,
            outf64.size,
            i_data,
            i_size,
            q_data,
            q_size,
            n,
            navg,
            nfft,
            window,
            fmt,
        )
    elif "int32" == dtype:
        result = _lib.gn_fft32(
            outf64,
            outf64.size,
            i_data,
            i_size,
            q_data,
            q_size,
            n,
            navg,
            nfft,
            window,
            fmt,
        )
    elif "int64" == dtype:
        result = _lib.gn_fft64(
            outf64,
            outf64.size,
            i_data,
            i_size,
            q_data,
            q_size,
            n,
            navg,
            nfft,
            window,
            fmt,
        )
    else:
        result = _lib.gn_fft(
            outf64, outf64.size, i_data, i_size, q_data, q_size, navg, nfft, window
        )
    _raise_exception_on_failure(result)
    return out


def rfft(a, *args):
    """
    Compute Real-FFT

    Args:
        ``a`` (``ndarray``) : Input array of type ``float64``, ``int16``, ``int32``, or ``int64``

        ``args`` (``list``) : Additional arguments
            1. If ``a`` is of type ``float64``, then compute the real-FFT of normalized samples with the following interpretation.

                ``navg`` (``int``) : FFT averaging number

                ``nfft`` (``int``) : FFT size

                ``window`` (``Window``): Window

                ``scale`` (``RfftScale``): Scaling mode

            2. If ``a`` is of type ``int16``, ``int32``, or ``int64``, then compute the FFT of interleaved quantized samples with the following interpretation.

                ``n`` (``int``) : Resolution (Bitwidth of a)

                ``navg`` (``int``) : FFT averaging number

                ``nfft`` (``int``) : FFT size

                ``window`` (``Window``): Window

                ``fmt`` (``CodeFormat``): Code format

                ``scale`` (``RfftScale``): Scaling mode

    Returns:
        ``out`` (``ndarray``) : FFT result of type ``float64`` with interleaved Re/Im components
    """
    dtype = _check_ndarray(a, ["float64", "int16", "int32", "int64"])
    nargs = len(args)
    if "float64" == dtype:
        # normalized samples
        navg = 1 if nargs == 0 else args[0]
        nfft = 0 if nargs <= 1 else args[1]
        window = Window.NO_WINDOW if nargs <= 2 else args[2]
        scale = RfftScale.DBFS_SIN if nargs <= 3 else args[3]
        n = None
        fmt = None
    else:
        # quantized samples
        if 0 == len(args):
            raise Exception("Missing required parameter, n, code width")
        n = args[0]
        navg = 1 if nargs <= 1 else args[1]
        nfft = 0 if nargs <= 2 else args[2]
        window = Window.NO_WINDOW if nargs <= 3 else args[3]
        fmt = CodeFormat.TWOS_COMPLEMENT if nargs <= 4 else args[4]
        scale = RfftScale.DBFS_SIN if nargs <= 5 else args[5]
    out_size = _c_size_t(0)
    navg = max(0, navg)
    nfft = max(0, nfft)
    result = _lib.gn_rfft_size(_ctypes.byref(out_size), a.size, navg, nfft)
    _raise_exception_on_failure(result)
    out = _np.empty(out_size.value // 2, dtype="complex128")
    outf64 = out.view("float64")
    if "int16" == dtype:
        result = _lib.gn_rfft16(
            outf64, outf64.size, a, a.size, n, navg, nfft, window, fmt, scale
        )
    elif "int32" == dtype:
        result = _lib.gn_rfft32(
            outf64, outf64.size, a, a.size, n, navg, nfft, window, fmt, scale
        )
    elif "int64" == dtype:
        result = _lib.gn_rfft64(
            outf64, outf64.size, a, a.size, n, navg, nfft, window, fmt, scale
        )
    else:
        result = _lib.gn_rfft(outf64, outf64.size, a, a.size, navg, nfft, window, scale)
    _raise_exception_on_failure(result)
    return out


"""
Fourier Utilities
"""

_lib.gn_alias.argtypes = [_c_double_p, _c_double, _c_double, _c_int]
_lib.gn_coherent.argtypes = [_c_double_p, _c_size_t, _c_double, _c_double]
_lib.gn_freq_axis.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _c_size_t,
    _c_int,
    _c_double,
    _c_int,
]
_lib.gn_freq_axis_size.argtypes = [_c_size_t_p, _c_size_t, _c_int]
_lib.gn_fftshift.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t]
_lib.gn_ifftshift.argtypes = [_ndptr_f64_1d, _c_size_t, _ndptr_f64_1d, _c_size_t]


def alias(fs, freq, axis_type):
    """Compute the aliased frequency of a signal.

    Maps the frequency into the Nyquist zone determined by axis_type:
    DC_CENTER maps to [-fs/2, fs/2), DC_LEFT maps to [0, fs), REAL maps
    to [0, fs/2].

    Parameters
    ----------
    fs : float
        Sample rate in Hz.
    freq : float
        Signal frequency in Hz.
    axis_type : FreqAxisType
        Frequency axis type determining the target Nyquist zone.

    Returns
    -------
    out : float
        Aliased frequency in Hz.

    """
    out = _c_double(0.0)
    result = _lib.gn_alias(_ctypes.byref(out), fs, freq, axis_type)
    _raise_exception_on_failure(result)
    return out.value


def coherent(nfft, fs, freq):
    """Compute the nearest coherent frequency for a given FFT size and sample rate.

    Coherent sampling ensures the signal lands exactly on an FFT bin,
    eliminating spectral leakage. For power-of-2 FFT sizes, the returned
    frequency corresponds to an odd number of cycles.

    Parameters
    ----------
    nfft : int
        FFT size.
    fs : float
        Sample rate in Hz.
    freq : float
        Desired signal frequency in Hz.

    Returns
    -------
    out : float
        Nearest coherent frequency in Hz.

    """
    out = _c_double(0.0)
    result = _lib.gn_coherent(_ctypes.byref(out), nfft, fs, freq)
    _raise_exception_on_failure(result)
    return out.value


def fftshift(a):
    """Shift the zero-frequency (DC) component to the center of the spectrum.

    Performs a circular shift by N/2, converting a DC-left spectrum [0, fs)
    to a DC-center spectrum [-fs/2, fs/2).

    Parameters
    ----------
    a : ndarray of dtype 'float64'
        Input array with DC at index 0.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array with DC shifted to center.

    """
    _check_ndarray(a, "float64")
    out = _np.empty(a.size, dtype="float64")
    result = _lib.gn_fftshift(out, out.size, a, a.size)
    _raise_exception_on_failure(result)
    return out


def freq_axis(nfft, axis_type, fs=1.0, axis_format=FreqAxisFormat.FREQ):
    """Generate a frequency axis array for plotting FFT results.

    Parameters
    ----------
    nfft : int
        FFT size.
    axis_type : FreqAxisType
        DC_CENTER, DC_LEFT, or REAL.
    fs : float
        Sample rate in Hz (default: 1.0).
    axis_format : FreqAxisFormat
        Output in BINS, FREQ (Hz), or NORM (normalized) (default: FREQ).

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array of frequency values.

    """
    out_size = _c_size_t()
    result = _lib.gn_freq_axis_size(_ctypes.byref(out_size), nfft, axis_type)
    _raise_exception_on_failure(result)
    out = _np.empty(out_size.value, dtype="float64")
    result = _lib.gn_freq_axis(out, out.size, nfft, axis_type, fs, axis_format)
    _raise_exception_on_failure(result)
    return out


def ifftshift(a):
    """Inverse of fftshift. Shift DC from center back to the left of the array.

    Converts a DC-center spectrum [-fs/2, fs/2) to a DC-left spectrum [0, fs).

    Parameters
    ----------
    a : ndarray of dtype 'float64'
        Input array with DC at center.

    Returns
    -------
    out : ndarray of dtype 'float64'
        Array with DC shifted to index 0.

    """
    _check_ndarray(a, "float64")
    out = _np.empty(a.size, dtype="float64")
    result = _lib.gn_ifftshift(out, out.size, a, a.size)
    _raise_exception_on_failure(result)
    return out


"""
Manager
"""

_lib.gn_mgr_compare.argtypes = [_c_bool_p, _c_char_p, _c_char_p]
_lib.gn_mgr_contains.argtypes = [_c_bool_p, _c_char_p]
_lib.gn_mgr_remove.argtypes = [_c_char_p]
_lib.gn_mgr_save.argtypes = [_c_char_p, _c_size_t, _c_char_p, _c_char_p]
_lib.gn_mgr_size.argtypes = [_c_size_t_p]
_lib.gn_mgr_to_string.argtypes = [_c_char_p, _c_size_t, _c_char_p]
_lib.gn_mgr_type.argtypes = [_c_char_p, _c_size_t, _c_char_p]


def mgr_clear():
    """Remove all objects from the global object manager."""
    _lib.gn_mgr_clear()


def mgr_compare(key1, key2):
    """Compare two managed objects for equality.

    Args:
        ``key1`` (``str``) : Key of the first object

        ``key2`` (``str``) : Key of the second object

    Returns:
        ``bool`` : True if both objects have identical configurations
    """
    equal = _c_bool(False)
    key1 = bytes(key1, "utf-8")
    key2 = bytes(key2, "utf-8")
    result = _lib.gn_mgr_compare(_ctypes.byref(equal), key1, key2)
    _raise_exception_on_failure(result)
    return equal.value


def mgr_contains(key):
    """Check whether an object with the given key exists in the manager.

    Args:
        ``key`` (``str``) : Object key to look up

    Returns:
        ``bool`` : True if the key exists in the manager
    """
    found = _c_bool(False)
    key = bytes(key, "utf-8")
    result = _lib.gn_mgr_contains(_ctypes.byref(found), key)
    _raise_exception_on_failure(result)
    return found.value


def mgr_remove(key):
    """Remove the object with the given key from the manager.

    Args:
        ``key`` (``str``) : Key of the object to remove
    """
    key = bytes(key, "utf-8")
    _lib.gn_mgr_remove(key)


def mgr_save(key, filename=""):
    """Serialize the managed object to a JSON file.

    Args:
        ``key`` (``str``) : Key of the object to save

        ``filename`` (``str``) : Output file path (if empty, a default name is generated from the key)

    Returns:
        ``str`` : The filename that was used
    """
    size = _c_size_t(0)
    key = bytes(key, "utf-8")
    filename = bytes(filename, "utf-8")
    result = _lib.gn_mgr_save_filename_size(_ctypes.byref(size), key, filename)
    _raise_exception_on_failure(result)
    buf = _ctypes.create_string_buffer(size.value)
    result = _lib.gn_mgr_save(buf, len(buf), key, filename)
    _raise_exception_on_failure(result)
    return buf.value.decode("utf-8")


def mgr_size():
    """Return the number of objects stored in the manager.

    Returns:
        ``int`` : Number of managed objects
    """
    size = _c_size_t(0)
    _lib.gn_mgr_size(_ctypes.byref(size))
    return size.value


def mgr_to_string(key=""):
    """Return a string representation of a managed object.

    If key is empty, returns a summary of all objects in the manager.

    Args:
        ``key`` (``str``) : Object key (if empty, summarizes all objects)

    Returns:
        ``str`` : String representation
    """
    size = _c_size_t(0)
    key = bytes(key, "utf-8")
    _lib.gn_mgr_to_string_size(_ctypes.byref(size), key)
    buf = _ctypes.create_string_buffer(size.value)
    result = _lib.gn_mgr_to_string(buf, len(buf), key)
    _raise_exception_on_failure(result)
    return buf.value.decode("utf-8")


def mgr_type(key):
    """Return the type name of the managed object.

    Args:
        ``key`` (``str``) : Object key

    Returns:
        ``str`` : Type name of the object
    """
    size = _c_size_t(0)
    key = bytes(key, "utf-8")
    result = _lib.gn_mgr_type_size(_ctypes.byref(size), key)
    _raise_exception_on_failure(result)
    buf = _ctypes.create_string_buffer(size.value)
    result = _lib.gn_mgr_type(buf, len(buf), key)
    _raise_exception_on_failure(result)
    return buf.value.decode("utf-8")


"""
Signal Processing
"""

_lib.gn_downsample.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _c_int,
    _c_bool,
]
_lib.gn_downsample16.argtypes = [
    _ndptr_i16_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _c_int,
    _c_bool,
]
_lib.gn_downsample32.argtypes = [
    _ndptr_i32_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _c_int,
    _c_bool,
]
_lib.gn_downsample64.argtypes = [
    _ndptr_i64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _c_int,
    _c_bool,
]
_lib.gn_fshift.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _c_double,
    _c_double,
]
_lib.gn_fshift16.argtypes = [
    _ndptr_i16_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _c_int,
    _c_double,
    _c_double,
    _c_int,
]
_lib.gn_fshift32.argtypes = [
    _ndptr_i32_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _c_int,
    _c_double,
    _c_double,
    _c_int,
]
_lib.gn_fshift64.argtypes = [
    _ndptr_i64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _c_int,
    _c_double,
    _c_double,
    _c_int,
]
_lib.gn_normalize16.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
    _c_int,
    _c_int,
]
_lib.gn_normalize32.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
    _c_int,
    _c_int,
]
_lib.gn_normalize64.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
    _c_int,
    _c_int,
]
_lib.gn_polyval.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
]
_lib.gn_quantize16.argtypes = [
    _ndptr_i16_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _c_double,
    _c_int,
    _c_double,
    _c_int,
]
_lib.gn_quantize32.argtypes = [
    _ndptr_i32_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _c_double,
    _c_int,
    _c_double,
    _c_int,
]
_lib.gn_quantize64.argtypes = [
    _ndptr_i64_1d,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
    _c_double,
    _c_int,
    _c_double,
    _c_int,
]


def downsample(a, ratio, interleaved=False):
    """Decimate a waveform by keeping every Nth sample.

    Keeps every ratio-th sample from the input. If interleaved is True, the
    input is treated as interleaved I/Q pairs and decimation preserves pair
    alignment.

    Args:
        ``a`` (``ndarray``) : Input array of type ``complex128``, ``float64``, ``int16``, ``int32``, or ``int64``

        ``ratio`` (``int``) : Decimation ratio (keep every Nth sample)

        ``interleaved`` (``bool``) : If True, treat ``a`` as interleaved I/Q data

    Returns:
        ``out`` (``ndarray``) : Downsampled waveform with same dtype as input
    """
    dtype = _check_ndarray(a, ["complex128", "float64", "int16", "int32", "int64"])
    ratio = int(ratio)
    if (
        "complex128" == dtype
    ):  # parameter 'interleaved' is ignored because 'complex128' is
        out_size = _c_size_t(0)
        af64 = a.view("float64")
        result = _lib.gn_downsample_size(
            _ctypes.byref(out_size), af64.size, ratio, True
        )
        _raise_exception_on_failure(result)
        out = _np.empty(out_size.value // 2, dtype="complex128")
        outf64 = out.view("float64")
        result = _lib.gn_downsample(
            outf64, outf64.size, af64, af64.size, ratio, interleaved
        )
    else:
        out_size = _c_size_t(0)
        result = _lib.gn_downsample_size(
            _ctypes.byref(out_size), a.size, ratio, interleaved
        )
        _raise_exception_on_failure(result)
        out = _np.empty(out_size.value, dtype=dtype)
        if "int16" == dtype:
            result = _lib.gn_downsample16(out, out.size, a, a.size, ratio, interleaved)
        elif "int32" == dtype:
            result = _lib.gn_downsample32(out, out.size, a, a.size, ratio, interleaved)
        elif "int64" == dtype:
            result = _lib.gn_downsample64(out, out.size, a, a.size, ratio, interleaved)
        else:
            result = _lib.gn_downsample(out, out.size, a, a.size, ratio, interleaved)
    _raise_exception_on_failure(result)
    return out


def fshift(a, *args):
    """
    Perform frequency shift

    Args:
        ``a`` (``ndarray``) : Input array of type ``complex128``, ``float64``, ``int16``, ``int32``, or ``int64``

        ``args`` (``list``) : Additional arguments
            1. If ``a`` is of type ``complex128`` or ``float64``, then perform frequency shift of interleaved normalized samples with the following interpretation.

                ``fs`` (``double``) : Sample rate

                ``fshift_`` (``double``) : Shift frequency

                In this case, if ``a`` is not complex, then a is interpreted to contain interleaved I/Q samples.

            2. If ``a`` is of type ``float64``, then perform frequency shift of split normalized samples.

                ``q`` (``float64``) : Quadrature component

                ``fs`` (``double``) : Sample rate

                ``fshift_`` (``double``) : Shift frequency

                In this case, ``a`` is interpreted to be the In-phase component.

            3. If ``a`` is of type ``int16``, ``int32``, or ``int64``, then perform frequency shift of interleaved quantized samples with the following interpretation.

                ``n`` (``int``) : Resolution (Bitwidth of a)

                ``fs`` (``double``) : Sample rate

                ``fshift_`` (``double``) : Shift frequency

                ``fmt`` (``CodeFormat``): Code format

                In this case, ``a`` is interpreted to contain interleaved quantized samples.

            4. If ``a`` is of type ``int16``, ``int32``, or ``int64``, then perform frequency shift of split quantized samples with the following interpretation.

                ``q`` (``int16``, ``int32``, or ``int64``) : Quadrature component

                ``n`` (``int``) : Resolution (Bitwidth of ``a``)

                ``fs`` (``double``) : Sample rate

                ``fshift_`` (``double``) : Shift frequency

                ``fmt`` (``CodeFormat``): Code format

                In this case, ``a`` is interpreted to to be the In-phase component.

    Returns:
        ``out`` (``ndarray``) : Frequency-shifted input waveform. The output datatype is the same as the input datatype.

    """
    dtype = _check_ndarray(a, ["complex128", "float64", "int16", "int32", "int64"])
    nargs = len(args)
    q_data = _np.empty(0, dtype=dtype)
    q_size = 0
    base_index = 0
    if dtype in ["complex128", "float64"]:  # normalized samples
        if "complex128" == dtype:
            i_data = a.view("float64")
            q_data.dtype = "float64"
        else:
            i_data = a
            if 0 < nargs and isinstance(args[0], _np.ndarray):  # arg[0] is Q data
                _check_ndarray(args[0], dtype)
                q_data = args[0]
                q_size = q_data.size
                base_index = 1
        i_size = i_data.size
        if nargs <= base_index + 1:
            raise Exception("Missing required parameters")
        fs = args[base_index]
        fshift_ = args[base_index + 1]
        out_size = _c_size_t(0)
        result = _lib.gn_fshift_size(_ctypes.byref(out_size), i_size, q_size)
        _raise_exception_on_failure(result)
        if "complex128" == dtype:
            out = _np.empty(out_size.value // 2, dtype=dtype)
            outf64 = out.view("float64")
            result = _lib.gn_fshift(
                outf64, outf64.size, i_data, i_size, q_data, q_size, fs, fshift_
            )
        else:
            out = _np.empty(out_size.value, dtype=dtype)
            result = _lib.gn_fshift(
                out, out.size, i_data, i_size, q_data, q_size, fs, fshift_
            )
    else:  # quantized samples
        i_data = a
        i_size = i_data.size
        if 0 < nargs and isinstance(args[0], _np.ndarray):  # arg[0] is Q data
            _check_ndarray(args[0], dtype)
            q_data = args[0]
            q_size = q_data.size
            base_index = 1
        if nargs <= base_index + 2:
            raise Exception("Missing required parameters")
        n = args[base_index]
        fs = args[base_index + 1]
        fshift_ = args[base_index + 2]
        fmt = (
            CodeFormat.TWOS_COMPLEMENT
            if nargs <= base_index + 3
            else args[base_index + 3]
        )
        out_size = _c_size_t(0)
        result = _lib.gn_fshift_size(_ctypes.byref(out_size), i_size, q_size)
        _raise_exception_on_failure(result)
        out = _np.empty(out_size.value, dtype=dtype)
        if "int16" == dtype:
            result = _lib.gn_fshift16(
                out, out.size, i_data, i_size, q_data, q_size, n, fs, fshift_, fmt
            )
        elif "int32" == dtype:
            result = _lib.gn_fshift32(
                out, out.size, i_data, i_size, q_data, q_size, n, fs, fshift_, fmt
            )
        else:  # 'int64'
            result = _lib.gn_fshift64(
                out, out.size, i_data, i_size, q_data, q_size, n, fs, fshift_, fmt
            )
    _raise_exception_on_failure(result)
    return out


def normalize(a, n, fmt=CodeFormat.TWOS_COMPLEMENT):
    """Convert quantized integer samples to normalized floating-point values in [-1, 1).

    For two's complement, scales by 2/2^n. For offset binary, subtracts the
    midpoint offset before scaling.

    Args:
        ``a`` (``ndarray``) : Input array of type ``int16``, ``int32``, or ``int64``

        ``n`` (``int``) : ADC resolution in bits

        ``fmt`` (``CodeFormat``) : Binary code format (default: TWOS_COMPLEMENT)

    Returns:
        ``out`` (``ndarray``) : ``float64`` array of normalized values in [-1, 1)
    """
    dtype = _check_ndarray(a, ["int16", "int32", "int64"])
    out = _np.empty(a.size, dtype="float64")
    if "int16" == dtype:
        result = _lib.gn_normalize16(out, out.size, a, a.size, n, fmt)
    elif "int32" == dtype:
        result = _lib.gn_normalize32(out, out.size, a, a.size, n, fmt)
    else:
        result = _lib.gn_normalize64(out, out.size, a, a.size, n, fmt)
    _raise_exception_on_failure(result)
    return out


def polyval(a, c):
    """Evaluate a polynomial at each element of the input array using Horner's method.

    Coefficients are ordered as [c0, c1, c2, ...] where
    y = c0 + c1*x + c2*x^2 + ... Commonly used to model nonlinear transfer
    function distortion.

    Args:
        ``a`` (``ndarray``) : Input array of type ``float64``

        ``c`` (``ndarray`` or ``list``) : Polynomial coefficients [c0, c1, c2, ...] of type ``float64``

    Returns:
        ``out`` (``ndarray``) : ``float64`` array of polynomial evaluation results
    """
    _check_ndarray(a, "float64")
    c = _np.array(c, dtype="float64")
    out = _np.empty(a.size, dtype="float64")
    result = _lib.gn_polyval(out, out.size, a, a.size, c, c.size)
    _raise_exception_on_failure(result)
    return out


def quantize16(a, fsr, n, noise=0.0, fmt=CodeFormat.TWOS_COMPLEMENT):
    """Quantize floating-point samples to 16-bit integer codes.

    The full-scale range determines the LSB size: LSB = fsr / 2^n. Samples
    are mapped to codes by floor(sample/LSB), clamped to the valid code range.
    Optional Gaussian noise can be added before quantization to model thermal noise.

    Args:
        ``a`` (``ndarray``) : Input array of type ``float64``

        ``fsr`` (``float``) : Full-scale range of the waveform

        ``n`` (``int``) : ADC resolution in bits

        ``noise`` (``float``) : RMS level of Gaussian noise to add before quantization (default: 0.0)

        ``fmt`` (``CodeFormat``) : Binary code format (default: TWOS_COMPLEMENT)

    Returns:
        ``out`` (``ndarray``) : ``int16`` array of quantized codes
    """
    _check_ndarray(a, "float64")
    out = _np.empty(a.size, dtype="int16")
    result = _lib.gn_quantize16(out, out.size, a, a.size, fsr, n, noise, fmt)
    _raise_exception_on_failure(result)
    return out


def quantize32(a, fsr, n, noise=0.0, fmt=CodeFormat.TWOS_COMPLEMENT):
    """Quantize floating-point samples to 32-bit integer codes.

    The full-scale range determines the LSB size: LSB = fsr / 2^n. Samples
    are mapped to codes by floor(sample/LSB), clamped to the valid code range.
    Optional Gaussian noise can be added before quantization to model thermal noise.

    Args:
        ``a`` (``ndarray``) : Input array of type ``float64``

        ``fsr`` (``float``) : Full-scale range of the waveform

        ``n`` (``int``) : ADC resolution in bits

        ``noise`` (``float``) : RMS level of Gaussian noise to add before quantization (default: 0.0)

        ``fmt`` (``CodeFormat``) : Binary code format (default: TWOS_COMPLEMENT)

    Returns:
        ``out`` (``ndarray``) : ``int32`` array of quantized codes
    """
    _check_ndarray(a, "float64")
    out = _np.empty(a.size, dtype="int32")
    result = _lib.gn_quantize32(out, out.size, a, a.size, fsr, n, noise, fmt)
    _raise_exception_on_failure(result)
    return out


def quantize64(a, fsr, n, noise=0.0, fmt=CodeFormat.TWOS_COMPLEMENT):
    """Quantize floating-point samples to 64-bit integer codes.

    The full-scale range determines the LSB size: LSB = fsr / 2^n. Samples
    are mapped to codes by floor(sample/LSB), clamped to the valid code range.
    Optional Gaussian noise can be added before quantization to model thermal noise.

    Args:
        ``a`` (``ndarray``) : Input array of type ``float64``

        ``fsr`` (``float``) : Full-scale range of the waveform

        ``n`` (``int``) : ADC resolution in bits

        ``noise`` (``float``) : RMS level of Gaussian noise to add before quantization (default: 0.0)

        ``fmt`` (``CodeFormat``) : Binary code format (default: TWOS_COMPLEMENT)

    Returns:
        ``out`` (``ndarray``) : ``int64`` array of quantized codes
    """
    _check_ndarray(a, "float64")
    out = _np.empty(a.size, dtype="int64")
    result = _lib.gn_quantize64(out, out.size, a, a.size, fsr, n, noise, fmt)
    _raise_exception_on_failure(result)
    return out


def quantize(a, fsr, n, noise=0.0, fmt=CodeFormat.TWOS_COMPLEMENT):
    """Quantize floating-point samples to integer codes with automatic bit-width selection.

    Selects ``int16`` output if n <= 16 (two's complement) or n < 16 (offset binary),
    and ``int32`` otherwise. See ``quantize16`` and ``quantize32`` for details.

    Args:
        ``a`` (``ndarray``) : Input array of type ``float64``

        ``fsr`` (``float``) : Full-scale range of the waveform

        ``n`` (``int``) : ADC resolution in bits

        ``noise`` (``float``) : RMS level of Gaussian noise to add before quantization (default: 0.0)

        ``fmt`` (``CodeFormat``) : Binary code format (default: TWOS_COMPLEMENT)

    Returns:
        ``out`` (``ndarray``) : ``int16`` or ``int32`` array of quantized codes
    """
    if n < 16 or (16 == n and CodeFormat.TWOS_COMPLEMENT == fmt):
        return quantize16(a, fsr, n, noise, fmt)
    else:
        return quantize32(a, fsr, n, noise, fmt)


"""
Waveforms
"""

_lib.gn_cos.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _c_double,
    _c_double,
    _c_double,
    _c_double,
    _c_double,
    _c_double,
]
_lib.gn_gaussian.argtypes = [_ndptr_f64_1d, _c_size_t, _c_double, _c_double]
_lib.gn_ramp.argtypes = [_ndptr_f64_1d, _c_size_t, _c_double, _c_double, _c_double]
_lib.gn_sin.argtypes = [
    _ndptr_f64_1d,
    _c_size_t,
    _c_double,
    _c_double,
    _c_double,
    _c_double,
    _c_double,
    _c_double,
]
_lib.gn_wf_analysis.argtypes = [
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _ndptr_f64_1d,
    _c_size_t,
]
_lib.gn_wf_analysis16.argtypes = [
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _ndptr_i16_1d,
    _c_size_t,
]
_lib.gn_wf_analysis32.argtypes = [
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _ndptr_i32_1d,
    _c_size_t,
]
_lib.gn_wf_analysis64.argtypes = [
    _c_char_p_p,
    _c_size_t,
    _c_double_p,
    _c_size_t,
    _ndptr_i64_1d,
    _c_size_t,
]


def cos(nsamples, fs, ampl, freq, phase=0.0, td=0.0, tj=0.0):
    """Generate a cosine waveform sampled at rate fs.

    The waveform is: ampl * cos(2*pi*freq*(t + td) + phase), where
    t = [0, 1/fs, 2/fs, ...]. Time delay td shifts the waveform in time.
    If tj > 0, random Gaussian aperture jitter is added to each sampling
    instant to model clock jitter.

    Args:
        ``nsamples`` (``int``) : Number of samples to generate

        ``fs`` (``float``) : Sample rate in S/s

        ``ampl`` (``float``) : Amplitude

        ``freq`` (``float``) : Signal frequency in Hz

        ``phase`` (``float``) : Phase offset in radians (default: 0.0)

        ``td`` (``float``) : Time delay in seconds (default: 0.0)

        ``tj`` (``float``) : RMS aperture jitter in seconds (default: 0.0)

    Returns:
        ``out`` (``ndarray``) : ``float64`` array of cosine waveform samples
    """
    out = _np.empty(nsamples, "float64")
    result = _lib.gn_cos(out, out.size, fs, ampl, freq, phase, td, tj)
    _raise_exception_on_failure(result)
    return out


def gaussian(nsamples, mean, sd):
    """Generate an array of Gaussian (normally distributed) random samples.

    Args:
        ``nsamples`` (``int``) : Number of samples to generate

        ``mean`` (``float``) : Mean of the distribution

        ``sd`` (``float``) : Standard deviation of the distribution

    Returns:
        ``out`` (``ndarray``) : ``float64`` array of Gaussian random samples
    """
    out = _np.empty(nsamples, "float64")
    result = _lib.gn_gaussian(out, out.size, mean, sd)
    _raise_exception_on_failure(result)
    return out


def ramp(nsamples, start, stop, noise):
    """Generate a linear ramp waveform from start to stop with midpoint sampling.

    Each sample is placed at the center of its step. If noise > 0, Gaussian
    noise with the specified RMS level is added.

    Args:
        ``nsamples`` (``int``) : Number of samples to generate

        ``start`` (``float``) : Start value

        ``stop`` (``float``) : Stop value

        ``noise`` (``float``) : RMS level of Gaussian noise to add (0 for no noise)

    Returns:
        ``out`` (``ndarray``) : ``float64`` array of ramp waveform samples
    """
    out = _np.empty(nsamples, "float64")
    result = _lib.gn_ramp(out, out.size, start, stop, noise)
    _raise_exception_on_failure(result)
    return out


def sin(nsamples, fs, ampl, freq, phase=0.0, td=0.0, tj=0.0):
    """Generate a sine waveform sampled at rate fs.

    The waveform is: ampl * sin(2*pi*freq*(t + td) + phase), where
    t = [0, 1/fs, 2/fs, ...]. Time delay td shifts the waveform in time.
    If tj > 0, random Gaussian aperture jitter is added to each sampling
    instant to model clock jitter.

    Args:
        ``nsamples`` (``int``) : Number of samples to generate

        ``fs`` (``float``) : Sample rate in S/s

        ``ampl`` (``float``) : Amplitude

        ``freq`` (``float``) : Signal frequency in Hz

        ``phase`` (``float``) : Phase offset in radians (default: 0.0)

        ``td`` (``float``) : Time delay in seconds (default: 0.0)

        ``tj`` (``float``) : RMS aperture jitter in seconds (default: 0.0)

    Returns:
        ``out`` (``ndarray``) : ``float64`` array of sine waveform samples
    """
    out = _np.empty(nsamples, "float64")
    result = _lib.gn_sin(out, out.size, fs, ampl, freq, phase, td, tj)
    _raise_exception_on_failure(result)
    return out


def wf_analysis(a):
    """Compute time-domain statistics of a waveform.

    Computes min, max, mid, range, average, RMS, and AC-RMS values.

    Args:
        ``a`` (``ndarray``) : Input array of type ``float``, ``int16``, ``int32``, or ``int64``

    Returns:
        ``results`` (``dict``) : Dictionary containing all waveform analysis results

    Notes:
        Every Key:Value pair in the dictionary is ``str``:``float``.

        The dictionary contains the following keys:
            ``signaltype`` : Signal type: 0=Real, 1=Complex

            ``min`` : Minumum value

            ``max`` : Maximum value

            ``mid`` : Middle value ((max + min) / 2)

            ``range`` : Range (max - min)

            ``avg`` : Average value

            ``rms`` : RMS value

            ``rmsac`` : RMS value with DC removed

            ``min_index`` : Index of first occurence of minimum value

            ``max_index`` : Index of first occurence of maximum value
    """
    dtype = _check_ndarray(a, ["float", "int16", "int32", "int64"])
    keys, values = _get_analysis_containers(_AnalysisType.WAVEFORM)
    if "int16" == dtype:
        result = _lib.gn_wf_analysis16(keys, len(keys), values, len(values), a, a.size)
    elif "int32" == dtype:
        result = _lib.gn_wf_analysis32(keys, len(keys), values, len(values), a, a.size)
    elif "int64" == dtype:
        result = _lib.gn_wf_analysis64(keys, len(keys), values, len(values), a, a.size)
    else:
        result = _lib.gn_wf_analysis(keys, len(keys), values, len(values), a, a.size)
    _raise_exception_on_failure(result)
    results = _make_results_dict(keys, values)
    return results
