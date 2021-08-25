/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/formatted_data.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_FORMATTED_DATA_HPP
#define ICD_ANALYSIS_FORMATTED_DATA_HPP

#include "analysis.hpp"
#include "types.hpp"
#include "var_map.hpp"

#define ICD_API ICD_ANALYSIS_DECL

namespace analysis { /// @{ @ingroup FormattedData

str_t table(const std::vector<str_vector>& header_rows,
    const std::vector<str_vector>& data_rows,
    int col_margin,
    bool show_border,
    bool show_col_sep);

ICD_API std::vector<str_vector> result_data(const var_map& data,
    const var_vector& spec);

ICD_API str_t result_table(const var_map& results,
    const var_vector& spec,
    int col_margin,
    bool show_border,
    bool show_col_sep);

/// @} FormattedData

} // namespace analysis

#undef ICD_API

#endif // ICD_ANALYSIS_FORMATTED_DATA_HPP
