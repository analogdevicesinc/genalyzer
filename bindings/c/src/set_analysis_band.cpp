#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void set_analysis_band(fft_analysis_wrapper* cfftobj)
{
    an::fourier_analysis* fftobj;
    fftobj = reinterpret_cast<an::fourier_analysis*>(cfftobj);
    fftobj->set_analysis_band("0.0", "fs");
}
}
