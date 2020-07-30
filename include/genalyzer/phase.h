/*
 * BSD 3-Clause License
 *
 * Copyright (c) 2020, Analog Devices, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/** @file phase.h
 *  @brief Function prototypes for phase related functions.
 *
 */

#ifndef INCLUDE_GENALYZER_PHASE_H_
#define INCLUDE_GENALYZER_PHASE_H_

#include <math.h>
#include <stdint.h>

/**
 * @brief Calculate phase difference between two complex double precision
 * signals.\n Signals must of of equal length.\n\n <b>Formula</b> : phase =
 * angle(chan0 * conj(chan1)) * 180 / &pi;\n
 *
 * @param chan0_i Array of the inphase (i) component of signal 1
 * @param chan0_q Array of the quadrature (q) component of signal 1
 * @param chan1_i Array of the inphase (i) component of signal 2
 * @param chan1_q Array of the quadrature (q) component of signal 2
 * @param samples Length of input arrays
 * @return phase difference between signals in degrees
 */
double phase_difference_cdouble(const double *chan0_i, const double *chan0_q,
                                const double *chan1_i, const double *chan1_q,
                                const uint16_t samples);

#endif /* INCLUDE_GENALYZER_PHASE_H_ */
