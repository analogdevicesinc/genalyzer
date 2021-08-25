#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void destroy_fft_analysis_wrapper(fft_analysis_wrapper* cfftobj)
{
    an::fft_analysis2* fftobj;
    fftobj = reinterpret_cast<an::fft_analysis2*>(cfftobj);
    delete (fftobj);
}
}
