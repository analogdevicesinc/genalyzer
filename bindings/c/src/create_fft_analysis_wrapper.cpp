#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
fft_analysis_wrapper* gn_create_fft_analysis_wrapper()
{
    an::fourier_analysis* fftobj = new an::fourier_analysis();
    return reinterpret_cast<fft_analysis_wrapper*>(fftobj);
}
}
