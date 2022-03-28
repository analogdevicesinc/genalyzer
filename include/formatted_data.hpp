#ifndef DCANALYSIS_IMPL_FORMATTED_DATA_HPP
#define DCANALYSIS_IMPL_FORMATTED_DATA_HPP

#include "type_aliases.hpp"

namespace dcanalysis_impl {

    str_t table(
        const std::vector<str_vector>& header_rows,
        const std::vector<str_vector>& data_rows,
        int col_margin,
        bool show_border,
        bool show_col_sep
    );

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_FORMATTED_DATA_HPP