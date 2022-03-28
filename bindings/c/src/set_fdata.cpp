#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void set_fdata(fft_analysis_wrapper* cfftobj, double fdata)
{
    an::fourier_analysis* fftobj;
    fftobj = reinterpret_cast<an::fourier_analysis*>(cfftobj);

    an::str_t fdata_str = an::real_to_string(fdata);
    fftobj->set_fdata(fdata_str);
}
}
