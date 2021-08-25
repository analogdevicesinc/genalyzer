/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/waveforms.cpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#include "waveforms.hpp"
#include "checks.hpp"
#include "constants.hpp"
#include <algorithm>
#include <functional>
#include <random>

namespace analysis {

namespace {

    void check_ampl(size_t ntones, real_t fsr, std::vector<real_t>& ampl)
    {
        if (ntones < ampl.size()) {
            throw base::exception("Size of amplitude spec > number of "
                                  "tones: "
                + std::to_string(ampl.size())
                + " > " + std::to_string(ntones));
        }
        if (ntones == ampl.size()) {
            for (real_t& a : ampl) {
                a = (fsr / 2) * std::pow(10.0, a / 20);
            }
        } else {
            if (1 < ntones && 1 < ampl.size()) {
                throw base::exception("If number of tones > 1, size of "
                                      "amplitude spec must be 0, 1, or "
                                      "equal to the number of tones");
            } else {
                real_t a = ampl.empty() ? 0.0 : ampl[0];
                a = (fsr / 2) * std::pow(10.0, a / 20) / ntones;
                ampl = std::vector<real_t>(ntones, a);
            }
        }
    }

    size_t check_freq(const std::vector<real_t>& freq)
    {
        for (real_t f : freq) {
            if (0.0 == f) {
                throw base::exception("Frequency must be non-zero");
            }
        }
        return freq.size();
    }

    void check_phase(size_t ntones, std::vector<real_t>& phase)
    {
        if (ntones < phase.size()) {
            throw base::exception("Size of phase spec > number of tones: "
                + std::to_string(phase.size())
                + " > " + std::to_string(ntones));
        }
        if (ntones == phase.size()) {
            for (real_t& p : phase) {
                p = std::fmod(p, k_2pi);
            }
        } else {
            if (1 < ntones && 1 < phase.size()) {
                throw base::exception("If number of tones > 1, size of "
                                      "phase spec must be 0, 1, or "
                                      "equal to the number of tones");
            } else {
                real_t p = phase.empty() ? 0.0 : phase[0];
                phase = std::vector<real_t>(ntones, std::fmod(p, k_2pi));
            }
        }
    }

    void generate_sinusoid(double (*func)(double),
        real_t* data,
        size_t size,
        real_t fs,
        real_t fsr,
        std::vector<real_t> ampl,
        const std::vector<real_t>& freq,
        std::vector<real_t> phase,
        real_t td,
        real_t tj,
        real_t offset)
    {
        check_array(data, size, "waveform array");
        check_fs(fs);
        check_fsr(fsr);
        size_t ntones = check_freq(freq);
        check_ampl(ntones, fsr, ampl);
        check_phase(ntones, phase);
        if (0 == ntones) {
            std::fill(data, data + size, offset);
            return;
        }
        // Generate time points
        const real_t tdfs = td * fs;
        if (0.0 == tj) {
            for (size_t i = 0; i < size; ++i) {
                data[i] = static_cast<real_t>(i) + tdfs;
            }
        } else {
            const real_t tjfs = std::fabs(tj) * fs;
            std::random_device rdev;
            std::mt19937 rgen(rdev());
            std::normal_distribution<double> rdist(tdfs, tjfs);
            auto delta_t = std::bind(rdist, rgen);
            for (size_t i = 0; i < size; ++i) {
                data[i] = static_cast<real_t>(i) + delta_t();
            }
        }
        // Generate tones
        if (ntones == 1) {
            const real_t twopift = k_2pi * freq[0] / fs;
            for (size_t i = 0; i < size; ++i) {
                data[i] = ampl[0] * func(twopift * data[i] + phase[0]);
            }
        } else if (ntones == 2) {
            const real_t twopif0t = k_2pi * freq[0] / fs;
            const real_t twopif1t = k_2pi * freq[1] / fs;
            for (size_t i = 0; i < size; ++i) {
                const real_t t = data[i];
                data[i] = ampl[0] * func(twopif0t * t + phase[0]);
                data[i] += ampl[1] * func(twopif1t * t + phase[1]);
            }
        } else {
            std::vector<real_t> twopift{};
            for (auto f : freq) {
                twopift.push_back(k_2pi * f / fs);
            }
            for (size_t i = 0; i < size; ++i) {
                const real_t t = data[i];
                data[i] = 0.0;
                for (size_t j = 0; j < ntones; ++j) {
                    data[i] += ampl[j] * func(twopift[j] * t + phase[j]);
                }
            }
        }
        // Offset
        if (0.0 != offset) {
            for (size_t i = 0; i < size; ++i) {
                data[i] += offset;
            }
        }
    }

} // namespace anonymous

void cos(real_t* data,
    size_t size,
    real_t fs,
    real_t fsr,
    const std::vector<real_t>& ampl,
    const std::vector<real_t>& freq,
    const std::vector<real_t>& phase,
    real_t td,
    real_t tj,
    real_t offset)
{
    generate_sinusoid(std::cos, data, size, fs, fsr,
        ampl, freq, phase, td, tj, offset);
}

void noise(real_t* data,
    size_t size,
    real_t fsr,
    real_t noise,
    real_t offset)
{
    check_array(data, size, "waveform data");
    check_fsr(fsr);
    real_t sd = (fsr / 2) * std::pow(10.0, noise / 20) / k_sqrt2;
    std::random_device rdev;
    std::mt19937 rgen(rdev());
    std::normal_distribution<real_t> rdist(offset, sd);
    auto normal = std::bind(rdist, rgen);
    for (size_t i = 0; i < size; ++i) {
        data[i] = normal();
    }
}

void ramp(real_t* data,
    size_t size,
    real_t start,
    real_t stop,
    real_t offset)
{
    check_array(data, size, "waveform data");
    if (0 == size) {
        return;
    }
    const real_t step = (stop - start) / static_cast<real_t>(size);
    real_t value = offset + start + step / 2;
    for (size_t i = 0; i < size; ++i) {
        data[i] = value;
        value += step;
    }
}

void sin(real_t* data,
    size_t size,
    real_t fs,
    real_t fsr,
    const std::vector<real_t>& ampl,
    const std::vector<real_t>& freq,
    const std::vector<real_t>& phase,
    real_t td,
    real_t tj,
    real_t offset)
{
    generate_sinusoid(std::sin, data, size, fs, fsr,
        ampl, freq, phase, td, tj, offset);
}

} // namespace analysis
