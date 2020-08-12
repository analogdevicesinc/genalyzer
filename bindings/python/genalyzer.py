"""
  BSD 3-Clause License

  Copyright (c) 2020, Analog Devices, Inc.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

  3. Neither the name of the copyright holder nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.
 """

import ctypes
from ctypes.util import find_library
import numpy as np

GENLIB = "genalyzer"
_lib = ctypes.CDLL(find_library(GENLIB), use_errno=True, use_last_error=True)
if not _lib._name:  # pylint: disable=W0212
    raise Exception("libgenalyzer not found")

# C to python mapping
_genalyzer_phase_difference_cdouble = _lib.phase_difference_cdouble
_genalyzer_phase_difference_cdouble.restype = ctypes.c_double
_genalyzer_phase_difference_cdouble.argtypes = (
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double),
    ctypes.POINTER(ctypes.c_double),
    ctypes.c_uint,
)

# Helpers
def _get_ptr_cpx(chan):
    real = np.copy(np.real(chan))
    imag = np.copy(np.imag(chan))
    real = real.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
    imag = imag.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
    return (real, imag)


# Python exports
def genalyzer_phase_difference_cdouble(chan0: np.ndarray, chan1: np.ndarray):
    """ Calculate difference phase between two complex signals

        Algorithm: phase = angle(chan0  conj(chan1))  180 / &pi

        Parameters
        ----------
        chan0 : np.complex128
            First complex signal
        chan1 : np.complex128
            Second complex signal

        Returns
        -------
        float
            Phase difference in degrees.
    """
    assert chan0.size == chan1.size, "Input arrays must be the same size"
    assert chan0.dtype == np.complex128, "Input 1 must of type np.complex128"
    assert chan1.dtype == np.complex128, "Input 2 must of type np.complex128"

    vlen = ctypes.c_uint(chan0.size)
    (real0, imag0) = _get_ptr_cpx(chan0)
    (real1, imag1) = _get_ptr_cpx(chan1)

    return _genalyzer_phase_difference_cdouble(real0, imag0, real1, imag1, vlen,)
