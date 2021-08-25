/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/fft_component.cpp $
Originator  : pderouni
Revision    : $Revision: 12274 $
Last Commit : $Date: 2020-02-19 06:47:51 -0500 (Wed, 19 Feb 2020) $
Last Editor : $Author: pderouni $
*/

#include "fft_component.hpp"
#include "enum_maps.hpp"
#include "io_map.hpp"

namespace analysis {

fft_component::unique_ptr fft_component::load(const io_map& state)
{
    auto& type_str = state.at(fft_comp_type_map.enum_name()).as_str();
    auto& tag_str = state.at(fft_comp_tag_map.enum_name()).as_str();
    auto tag = fft_comp_tag_map[tag_str];
    switch (fft_comp_type_map[type_str]) {
    case FFTCompType::Band: {
        auto& center = state.at("center").as_str();
        auto& width = state.at("width").as_str();
        bool keep_out = state.at("keep_out").as_bool();
        bool omit_tones = state.at("omit_tones").as_bool();
        return fft_band::create(tag, center, width, keep_out, omit_tones);
    }
    case FFTCompType::FixedTone: {
        auto& freq = state.at("freq").as_str();
        auto ssb = static_cast<int>(state.at("ssb").as_int());
        return fft_fixed_tone::create(tag, freq, ssb);
    }
    case FFTCompType::MaxTone: {
        auto& center = state.at("center").as_str();
        auto& width = state.at("width").as_str();
        auto ssb = static_cast<int>(state.at("ssb").as_int());
        return fft_max_tone::create(tag, center, width, ssb);
    }
    default:
        throw base::exception("No implementation for FFT component type \""
            + type_str + "\"");
    }
}

std::unique_ptr<io_map> fft_component::save() const
{
    auto state = io_map::create();
    state->emplace(fft_comp_type_map.enum_name(),
        fft_comp_type_map[type()]);
    state->emplace(fft_comp_tag_map.enum_name(),
        fft_comp_tag_map[m_tag]);
    save_state(*state);
    return state;
}

fft_component::str_vector
fft_component::table_row(int i, const str_t& key) const
{
    str_vector data{ std::to_string(i), key };
    data.push_back(fft_comp_type_map[type()]);
    data.push_back(fft_comp_tag_map[m_tag]);
    data.push_back(param_str());
    return data;
}

str_t fft_band::param_str() const
{
    return "C= " + m_center + " , W= " + m_width
        + " , KO= " + (m_keep_out ? 'T' : 'F')
        + " , OT= " + (m_omit_tones ? 'T' : 'F');
}

void fft_band::save_state(io_map& state) const
{
    state.emplace("center", m_center);
    state.emplace("width", m_width);
    state.emplace("keep_out", m_keep_out);
    state.emplace("omit_tones", m_omit_tones);
}

str_t fft_fixed_tone::param_str() const
{
    return "F= " + m_freq + " , SSB= " + std::to_string(m_ssb);
}

void fft_fixed_tone::save_state(io_map& state) const
{
    state.emplace("freq", m_freq);
    state.emplace("ssb", static_cast<io_int_t>(m_ssb));
}

str_t fft_max_tone::param_str() const
{
    return "C= " + m_center + " , W= " + m_width
        + " , SSB= " + std::to_string(m_ssb);
}

void fft_max_tone::save_state(io_map& state) const
{
    state.emplace("center", m_center);
    state.emplace("width", m_width);
    state.emplace("ssb", static_cast<io_int_t>(m_ssb));
}

} // namespace analysis
