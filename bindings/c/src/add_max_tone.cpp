#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
fft_analysis_wrapper* add_max_tone(fft_analysis_wrapper* cfftobj, const char* tone_label)
{
    an::fft_analysis2* fftobj;
    fftobj = reinterpret_cast<an::fft_analysis2*>(cfftobj);
    fftobj->add_max_tone(tone_label, an::FFTCompTag::Signal, "0.0", "fs", 5);
}
}
