#include "cgenalyzer.h"
#include "cgenalyzer_private.h"

extern "C" {
void config_tone_meas(config_tone* c, meas_domain m_domain, waveform_type wf_type, size_t fft_order, int num_avgs, double sample_rate, double full_scale_range, int resolution, bool fsample_update, bool fdata_update, bool fshift_update)
{
    double z[1] = { 0.0 };
    config_tone_gen(c,
        m_domain,
        wf_type,
        fft_order, // FFT order
        num_avgs, // # of FFTs averaged
        sample_rate, // sample rate
        full_scale_range, // full-scale range
        resolution, // ADC resolution: unused configuration setting
        z, // tone frequency, # of array elements = num_tones
        z, // tone scale, # of array elements = num_tones
        z, // tone phase, # of array elements = num_tones
        1, // # of tones
        fsample_update,
        fdata_update,
        fshift_update);
}
}
