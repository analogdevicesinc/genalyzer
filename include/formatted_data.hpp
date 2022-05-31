#ifndef GENALYZER_IMPL_FORMATTED_DATA_HPP
#define GENALYZER_IMPL_FORMATTED_DATA_HPP

#include "type_aliases.hpp"

namespace genalyzer_impl {

    str_t table(
        const std::vector<str_vector>& header_rows,
        const std::vector<str_vector>& data_rows,
        int col_margin,
        bool show_border,
        bool show_col_sep
    );

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_FORMATTED_DATA_HPP