// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#include "fourier_analysis.hpp"

#include <json.hpp>

#include "enum_maps.hpp"
#include "utils.hpp"
#include "version.hpp"

#include <fstream>
#include <iomanip>

using json = nlohmann::json;

namespace genalyzer_impl {

std::shared_ptr<fourier_analysis> fourier_analysis::load(const str_t &filename) {
	std::ifstream ifs(filename);
	if (!ifs.is_open()) {
		throw runtime_error("unable to open file '" + filename + "'");
	}
	try {
		json j;
		ifs >> j; // error handling?
		std::shared_ptr<fourier_analysis> p =
				std::make_shared<fourier_analysis>();
		j["en_conv_offset"].get_to(p->en_conv_offset);
		j["en_fund_images"].get_to(p->en_fund_images);
		j["en_quad_errors"].get_to(p->en_quad_errors);
		j["clk_as_noise"].get_to(p->clk_as_noise);
		j["dc_as_dist"].get_to(p->dc_as_dist);
		j["ilv_as_noise"].get_to(p->ilv_as_noise);
		p->set_analysis_band(j["ab_center"].get<str_t>(),
				j["ab_width"].get<str_t>());
		p->set_fdata(j["fdata"].get<str_t>());
		p->set_fsample(j["fsample"].get<str_t>());
		p->set_fshift(j["fshift"].get<str_t>());
		p->set_hd(j["hd"].get<int>());
		p->set_imd(j["imd"].get<int>());
		p->set_wo(j["wo"].get<int>());
		p->set_ssb(FASsb::Default, j["ssb_def"].get<int>());
		p->set_ssb(FASsb::DC, j["ssb_dc"].get<int>());
		p->set_ssb(FASsb::Signal, j["ssb_sig"].get<int>());
		p->set_ssb(FASsb::WO, j["ssb_wo"].get<int>());
		p->set_clk(j["clk"].get<std::set<int>>());
		p->set_ilv(j["ilv"].get<std::set<int>>());
		// components
		for (const json &jcomp : j["user_comps"]) {
			str_t key = jcomp["key"].get<str_t>();
			str_t type_str = jcomp["type"].get<str_t>();
			str_t tag_str = jcomp["tag"].get<str_t>();
			switch (static_cast<FACompType>(
					fa_comp_type_map.at(type_str))) {
				case FACompType::FixedTone: {
					FACompTag tag = static_cast<FACompTag>(
							fa_comp_tag_map.at(tag_str));
					str_t freq = jcomp["freq"].get<str_t>();
					int ssb = jcomp["ssb"].get<int>();
					p->add_fixed_tone(key, tag, freq, ssb);
					break;
				}
				case FACompType::MaxTone: {
					FACompTag tag = static_cast<FACompTag>(
							fa_comp_tag_map.at(tag_str));
					int ssb = jcomp["ssb"].get<int>();
					p->add_max_tone(key, tag, "0", "fdata", ssb);
					break;
				}
				default:
					throw std::exception();
			}
		}
		// variables
		var_map vars;
		j["user_vars"].get_to<var_map>(vars);
		for (const var_map::value_type &kv : vars) {
			p->set_var(kv.first, kv.second);
		}
		return p;
	} catch (const std::exception &) {
		throw runtime_error(
				"error loading fourier_analysis object from file '" +
				filename + "'");
	}
}

void fourier_analysis::save_impl(const str_t &filename) const {
	std::ofstream ofs(filename);
	if (!ofs.is_open()) {
		throw runtime_error("Unable to open file '" + filename + "'");
	}
	json jcomps;
	for (const str_t &key : m_user_keys) {
		const fourier_analysis_component &comp = *m_user_comps.at(key);
		switch (comp.type) {
			case FACompType::FixedTone: {
				auto c = static_cast<const fa_fixed_tone &>(comp);
				jcomps.push_back(json{
						{ "freq", c.freq },
						{ "key", key },
						{ "ssb", c.ssb },
						{ "tag", fa_comp_tag_map.at(to_int(c.tag)) },
						{ "type",
								fa_comp_type_map.at(to_int(c.type)) } });
				break;
			}
			case FACompType::MaxTone: {
				auto c = static_cast<const fa_max_tone &>(comp);
				jcomps.push_back(json{
						{ "key", key },
						{ "ssb", c.ssb },
						{ "tag", fa_comp_tag_map.at(to_int(c.tag)) },
						{ "type",
								fa_comp_type_map.at(to_int(c.type)) } });
				break;
			}
			default:
				continue;
		}
	}
	json j;
	j["ab_center"] = m_ab_center;
	j["ab_width"] = m_ab_width;
	j["clk"] = m_clk;
	j["clk_as_noise"] = clk_as_noise;
	j["dc_as_dist"] = dc_as_dist;
	j["en_conv_offset"] = en_conv_offset;
	j["en_fund_images"] = en_fund_images;
	j["en_quad_errors"] = en_quad_errors;
	j["fdata"] = m_fdata;
	j["fsample"] = m_fsample;
	j["fshift"] = m_fshift;
	j["hd"] = m_hd;
	j["ilv"] = m_ilv;
	j["ilv_as_noise"] = ilv_as_noise;
	j["imd"] = m_imd;
	j["ssb_dc"] = m_ssb_dc;
	j["ssb_def"] = m_ssb_def;
	j["ssb_sig"] = m_ssb_sig;
	j["ssb_wo"] = m_ssb_wo;
	j["user_comps"] = jcomps;
	j["user_vars"] = m_user_vars;
	j["version"] = version_string();
	j["wo"] = m_wo;
	ofs << std::setw(4) << j << std::endl;
}

} // namespace genalyzer_impl