#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void add_fixed_tone(fft_analysis_wrapper* cfftobj)
{
    an::fft_analysis2* fftobj;
    fftobj = reinterpret_cast<an::fft_analysis2*>(cfftobj);
    fftobj->add_fixed_tone("b", an::FFTCompTag::Signal, "2400000000", 0);
}
}
