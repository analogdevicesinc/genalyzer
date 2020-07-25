/** @file phase.h
 *  @brief Function prototypes for phase related functions.
 *
 */

#ifndef _PHASE_H
#define _PHASE_H

#include <math.h>
#include <stdint.h>

/**
 * @brief Calculate phase difference between two complex double precision signals.\n
 *        Signals must of of equal length.\n\n
 *        <b>Formula</b> : phase = angle(chan0 * conj(chan1)) * 180 / &pi;\n
 *
 * @param chan0_i Array of the inphase (i) component of signal 1
 * @param chan0_q Array of the quadrature (q) component of signal 1
 * @param chan1_i Array of the inphase (i) component of signal 2
 * @param chan1_q Array of the quadrature (q) component of signal 2
 * @param samples Length of input arrays
 * @return phase difference between signals in degrees
 */
double phase_difference_cdouble(const double *chan0_i, const double *chan0_q,
                                const double *chan1_i, const double *chan1_q, const uint16_t samples);

#endif /* _PHASE_H */
