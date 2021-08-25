#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
fft_analysis_wrapper* set_fdata(fft_analysis_wrapper* cfftobj, double fdata)
{
    an::fft_analysis2* fftobj;
    fftobj = reinterpret_cast<an::fft_analysis2*>(cfftobj);

    an::str_t fdata_str = an::real_to_string(fdata);
    fftobj->set_fdata(fdata_str);
}
}
