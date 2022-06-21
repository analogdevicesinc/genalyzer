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


#ifndef CGENALYZER_PRIVATE_H
#define CGENALYZER_PRIVATE_H
#include "cgenalyzer.h"
#include "cgenalyzer_advanced.h"

#include <array_ops.hpp>
#include <code_density.hpp>
#include <enum_maps.hpp>
#include <fourier_analysis.hpp>
#include <fourier_transforms.hpp>
#include <fourier_utilities.hpp>
#include <manager.hpp>
#include <processes.hpp>
#include <utils.hpp>
#include <version.hpp>
#include <waveforms.hpp>

#include <cstring>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>

constexpr int gn_success = 0;
constexpr int gn_failure = 1;

namespace gn = ::genalyzer_impl;

namespace util {

    class log
    {
    public:

        log()
            : m_log {},
              m_flag {false}
        {}

    public:

        bool check()
        {
            return m_flag;
        }

        void clear()
        {
            m_log.clear();
            m_flag = false;
        }
    
        std::string_view get() const
        {
            return m_log;
        }
    
        void set(const char* msg)
        {
            m_log = msg;
            m_flag = true;
        }
    
        size_t size() const
        {
            return m_log.size();
        }
    
    public:

        void append() {}

        template<typename... Types>
        void append(const char* s, Types... the_rest)
        {
            m_log.append(s);
            append(the_rest...);
        }
    
        void prepend() {}
    
        template<typename... Types>
        void prepend(const char* s, Types... the_rest)
        {
            prepend(the_rest...);
            m_log.insert(0, s);
        }
    
    private:
    
        std::string m_log;
        bool m_flag;

    }; // class log

    static log gn_error_log;

    static bool gn_null_terminate = true;

    template<typename... Types>
    int return_on_exception(const char* s, Types... the_rest)
    {
        gn_error_log.set(s);
        gn_error_log.append(the_rest...);
        return gn_failure;
    }

    template<typename T>
    int check_pointer(const T* p)
    {
        if (nullptr == p) {
            throw std::runtime_error("check_pointer : pointer is NULL");
        }
        return gn_success;
    }

    size_t terminated_size(size_t string_size);
    void fill_string_buffer(
        const char* src,        // Pointer to source
        size_t src_size,        // Size of source; should not count null-terminator, if it exists
        char* dst,              // Pointer to destination
        size_t dst_size         // Size of destination
        );
    std::string get_object_key_from_filename(const std::string& filename);
}

#ifdef __cplusplus
extern "C" {
#endif

using namespace genalyzer_impl;

struct gn_config_private {
    //meas_domain md = TIME;
    waveform_type wf_type;    
    gn::size_t nfft;    
    gn::size_t navg;    
    gn::size_t npts;

    gn::real_t sample_rate;
    gn::real_t data_rate;
    gn::real_t shift_freq;
    
    gn::real_t fsr;
    int qres;
    gn::real_t qnoise;
    
    char **obj_key;
    size_t num_obj_keys = 0;

    char **comp_key;
    size_t num_comp_keys = 0;

    // Number of single-side bins
    int ssb_fund = 0;
    int ssb_rest = 0;

    int max_harm_order = 3;
    /*
    int64_t min_code, max_code;
    gn::real_t irnoise;
    gn::real_t noise_pwr_db;*/
    GnCodeFormat code_format;
    GnWindow win;
    /*
    gn::DnlSignal dnl_type;
    double ramp_start, ramp_stop;*/

    // triplet to describe tones
    gn::real_t *freq;
    gn::real_t *scale;
    gn::real_t *phase;
    gn::size_t num_tones = 0;

    char **rkeys;
    double *rvalues;
    size_t *rkey_sizes;
    /*
    gn::size_t num_bins;
    gn::size_t num_hits;*/
};

#ifdef __cplusplus
}
#endif

#endif
