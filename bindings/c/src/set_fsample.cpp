#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void set_fsample(fft_analysis_wrapper* cfftobj, double fs)
{
    an::fourier_analysis* fftobj;
    fftobj = reinterpret_cast<an::fourier_analysis*>(cfftobj);

    an::str_t fs_str = an::real_to_string(fs);
    fftobj->set_fsample(fs_str);
}
}
