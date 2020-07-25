#include <math.h>
#include <stdint.h>

double phase_difference_cdouble(const double *chan0_i, const double *chan0_q,
                                const double *chan1_i, const double *chan1_q, const uint16_t samples)
{
    /*
    Measure phase between two complex signals.
    Instanteous phase at each point between signals
    is measured then averaged. This measurement assumes
    no sample offset between signals

    Fast phase estimation with complex signals handling wrapped phase
    */

    int k = 0;
    double real = 0, imag = 0;
    for (; k < samples; k++) {
        real += ((double)chan0_i[k] * (double)chan1_i[k] + (double)chan0_q[k] *
                 (double)chan1_q[k]);
        imag += ((double)chan0_i[k] * (double)chan1_q[k] - (double)chan0_q[k] *
                 (double)chan1_i[k]);
    }
    return atan2(imag, real) * 180/M_PI;
}
