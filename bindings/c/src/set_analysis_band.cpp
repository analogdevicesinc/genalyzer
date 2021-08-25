#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
fft_analysis_wrapper* set_analysis_band(fft_analysis_wrapper* cfftobj)
{
    an::fft_analysis2* fftobj;
    fftobj = reinterpret_cast<an::fft_analysis2*>(cfftobj);
    fftobj->set_analysis_band("0.0", "fs");
}
}
