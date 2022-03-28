#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void add_fixed_tone(fft_analysis_wrapper* cfftobj)
{
    an::fourier_analysis* fftobj;
    fftobj = reinterpret_cast<an::fourier_analysis*>(cfftobj);
    fftobj->add_fixed_tone("b", an::FACompTag::Signal, "2400000000", 0);
}
}
