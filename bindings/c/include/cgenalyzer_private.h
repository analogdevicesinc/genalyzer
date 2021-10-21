/*
 * cgenalyzer_private - genalyzer private header file
 *
 * Copyright (C) 2021 Analog Devices, Inc.
 * Author: Srikanth Pagadarai
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * */


#ifndef GENALYZER_PRIVATE_H
#define GENALYZER_PRIVATE_H
#include "cgenalyzer.h"
#include <code_density.hpp>
#include <fft_analysis2.hpp>
#include <real_to_string.hpp>

#ifdef __cplusplus
extern "C" {
#endif

using namespace analysis;
namespace an = ::analysis;

struct gn_config_s {
    meas_domain md = TIME;
    waveform_type wf_type;
    int_repr i_repr;
    an::size_t nfft;
    an::size_t in_stride = 1;
    int navg;
    an::size_t npts;

    an::real_t fs;
    an::real_t fdata;
    an::real_t fshift;
    bool update_fsample, update_fdata, update_fshift;
    an::real_t fsr;

    int res;
    int64_t min_code, max_code;
    an::real_t irnoise;
    an::real_t noise_pwr_db;
    an::CodeFormat cf;
    an::WindowType win;
    an::PmfType pmf_type;
    double ramp_start, ramp_stop;

    // triplet to describe tones
    an::real_t* freq;
    an::real_t* scale;
    an::real_t* phase;
    an::size_t n_tones = 0;

    an::size_t num_bins;
    an::size_t num_hits;
};

#ifdef __cplusplus
}
#endif

#endif
