#include "genalyzer/genalyzer.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <math.h>

#define LEN 1024
#define TOL 0.000001
int main()
{

    // Create two complex signals with different phase differences
    double i0[LEN];
    double q0[LEN];
    double i1[LEN];
    double q1[LEN];
    int k;
    double t, phase_est;

    double f_c = 1000;
    double f_s = 100000;
    double phase_degrees = 0;

    while (phase_degrees < 180) {
        for (k=0; k<LEN; k++) {

            t = k/(f_s);

            i0[k] = sin(2*M_PI*f_c*t);
            q0[k] = cos(2*M_PI*f_c*t);
            i1[k] = sin(2*M_PI*f_c*t - phase_degrees*M_PI/180);
            q1[k] = cos(2*M_PI*f_c*t - phase_degrees*M_PI/180);

        }
        phase_est = phase_difference_cdouble(i0, q0, i1, q1, LEN);
        printf("Phase Estimation: %f (Truth: %f)\n",phase_est, phase_degrees);

        if (fabs(phase_degrees-phase_est) > TOL)
          return -1;

        phase_degrees += 0.001;

    }


}
