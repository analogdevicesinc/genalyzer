#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
fft_analysis_wrapper* set_fshift(fft_analysis_wrapper* cfftobj, double fshift)
{
    an::fft_analysis2* fftobj;
    fftobj = reinterpret_cast<an::fft_analysis2*>(cfftobj);

    an::str_t fshift_str = an::real_to_string(fshift);
    fftobj->set_fshift(fshift_str);
}
}
