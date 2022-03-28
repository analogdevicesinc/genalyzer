#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void set_fshift(fft_analysis_wrapper* cfftobj, double fshift)
{
    an::fourier_analysis* fftobj;
    fftobj = reinterpret_cast<an::fourier_analysis*>(cfftobj);

    an::str_t fshift_str = an::real_to_string(fshift);
    fftobj->set_fshift(fshift_str);
}
}
