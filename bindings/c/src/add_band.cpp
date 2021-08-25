#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
fft_analysis_wrapper* add_band(fft_analysis_wrapper* cfftobj)
{
    an::fft_analysis2* fftobj;
    fftobj = reinterpret_cast<an::fft_analysis2*>(cfftobj);
    fftobj->add_band("a", an::FFTCompTag::Signal, "0.0", "fs", 0, 0);
}
}
