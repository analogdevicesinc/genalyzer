#include "formatted_data.hpp"

#include <algorithm>
#include <iomanip>
#include <sstream>

namespace genalyzer_impl {

    str_t table(
        const std::vector<str_vector>& header_rows,
        const std::vector<str_vector>& data_rows,
        int col_margin,
        bool show_border,
        bool show_col_sep
    )
    {
        std::vector<str_vector> all_rows (header_rows);
        all_rows.insert(all_rows.end(), data_rows.begin(), data_rows.end());
        // Determine number of columns
        size_t ncols = 0;
        for (const str_vector& row : all_rows) {
            ncols = std::max(ncols, row.size());
        }
        if (0 == ncols) {
            return "";
        }
        // Determine max data width in each column
        std::vector<size_t> col_widths (ncols, 0);
        for (str_vector& row : all_rows) {
            for (size_t col = 0; col < row.size(); ++col) {
                col_widths[col] = std::max(col_widths[col], row[col].size());
            }
            // Add missing columns to row
            row.resize(ncols, "");
        }
        // Setup
        col_margin = std::max<int>(0, std::min<int>(col_margin, 9));
        str_t col_pad = str_t(static_cast<size_t>(col_margin), ' ');
        str_t col_div = col_pad + (show_col_sep ? '|' : ' ') + col_pad;
        str_t left_border = (show_border ? '|' : ' ') + col_pad;
        str_t right_border = col_pad + (show_border ? '|' : ' ');
        size_t total_width = left_border.size() + (ncols - 1) * col_div.size() + right_border.size();
        for (size_t w : col_widths) {
            total_width += w;
        }
        str_t h_border = show_border ? (str_t(total_width, '=') + '\n') : "";
        str_t h_div = header_rows.empty() ? "" : (str_t(total_width, '=') + '\n');
        // Generate table
        std::ostringstream ss;
        ss << h_border;
        for (size_t row = 0; row < all_rows.size(); ++row) {
            if (header_rows.size() == row) {
                ss << h_div;
            }
            ss << left_border;
            const str_vector& data = all_rows[row];
            for (size_t col = 0; col < ncols; ++col) {
                ss << std::setw(static_cast<int>(col_widths[col]))
                   << std::left
                   << data[col];
                if (col + 1 < ncols) {
                    ss << col_div;
                }
            }
            ss << right_border << '\n';
        }
        ss << h_border;
        return ss.str();
    }

} // namespace genalyzer_impl
