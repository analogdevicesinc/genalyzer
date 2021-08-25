#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
fft_analysis_wrapper* create_fft_analysis_wrapper()
{
    an::fft_analysis2* fftobj = new an::fft_analysis2();
    return reinterpret_cast<fft_analysis_wrapper*>(fftobj);
}
}
