#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void gn_destroy_fft_analysis_wrapper(fft_analysis_wrapper* cfftobj)
{
    an::fourier_analysis* fftobj;
    fftobj = reinterpret_cast<an::fourier_analysis*>(cfftobj);
    delete (fftobj);
}
}
