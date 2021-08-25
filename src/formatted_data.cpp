/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/formatted_data.cpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#include "formatted_data.hpp"
#include "enum_maps.hpp"
#include "fft_analysis2.hpp"
#include <iomanip>

namespace analysis { /// @{ @ingroup FormattedData

str_t table(const std::vector<str_vector>& header_rows,
    const std::vector<str_vector>& data_rows,
    int col_margin,
    bool show_border,
    bool show_col_sep)
{
    std::vector<str_vector> all_rows(header_rows);
    all_rows.insert(all_rows.end(), data_rows.begin(), data_rows.end());
    // Determine number of columns
    size_t ncols = 0;
    for (const auto& row : all_rows) {
        ncols = std::max(ncols, row.size());
    }
    if (0 == ncols) {
        return "";
    }
    // Determine max data width in each column
    std::vector<size_t> width(ncols, 0);
    for (auto& row : all_rows) {
        for (size_t c = 0; c < row.size(); ++c) {
            width[c] = std::max(width[c], row[c].size());
        }
        // fill missing columns
        row.resize(ncols, "");
    }
    // Setup
    col_margin = std::max<int>(0, std::min<int>(col_margin, 9));
    auto colmarg = static_cast<size_t>(col_margin);
    str_t colpad = str_t(colmarg, ' ');
    str_t lbord = (show_border ? "|" : " ") + colpad;
    str_t coldiv = colpad + (show_col_sep ? "|" : " ") + colpad;
    str_t rbord = colpad + (show_border ? "|" : " ");
    size_t wtot = lbord.size() + (ncols - 1) * coldiv.size() + rbord.size();
    for (const auto& w : width) {
        wtot += w;
    }
    str_t hbord = show_border ? str_t(wtot, '=') + "\n" : "";
    str_t hdiv = header_rows.empty() ? "" : str_t(wtot, '=') + "\n";
    // Generate table
    std::ostringstream oss;
    oss << hbord;
    for (size_t r = 0; r < all_rows.size(); ++r) {
        if (header_rows.size() == r) {
            oss << hdiv;
        }
        oss << lbord;
        const str_vector& data = all_rows[r];
        for (size_t c = 0; c < ncols; ++c) {
            oss << std::setw(static_cast<int>(width[c]))
                << std::left
                << data[c];
            if (c + 1 < ncols) {
                oss << coldiv;
            }
        }
        oss << rbord << '\n';
    }
    oss << hbord;
    return oss.str();
}
std::vector<str_vector> result_data(const var_map& data,
    const var_vector& spec)
{
    const str_t& an_type_str = data.as_str(analysis_type_map.enum_name());
    switch (analysis_type_map[an_type_str]) {
    case AnalysisType::FFT:
        return fft_analysis2::result_data(data, spec);
    case AnalysisType::Waveform:
        break;
    default:
        break;
    }
    return std::vector<str_vector>();
}

str_t result_table(const var_map& results,
    const var_vector& spec,
    int col_margin,
    bool show_border,
    bool show_col_sep)
{
    return table({ { "Name", "Value", "Units" } },
        result_data(results, spec),
        col_margin,
        show_border,
        show_col_sep);
}

} // namespace analysis
