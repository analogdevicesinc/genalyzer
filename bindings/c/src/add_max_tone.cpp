#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void add_max_tone(fft_analysis_wrapper* cfftobj, const char* tone_label)
{
    an::fourier_analysis* fftobj;
    fftobj = reinterpret_cast<an::fourier_analysis*>(cfftobj);
    fftobj->add_max_tone(tone_label, an::FACompTag::Signal, "0.0", "fs", 5);
}
}
