#include "fourier_analysis_comp_mask.hpp"

#include "exceptions.hpp"
#include "utils.hpp"

#include <algorithm>

namespace genalyzer_impl {

    fourier_analysis_comp_mask::fourier_analysis_comp_mask(
        bool cplx, size_t array_size, const std::vector<size_t>& init)
        : m_mode {cplx ? Wrap : Stop},
          m_ufirst {0},
          m_ulast {array_size - 1},
          m_usize {array_size},
          m_first {0},
          m_last {static_cast<diff_t>(array_size - 1)},
          m_size {static_cast<diff_t>(array_size)},
          m_data (init)
    {
        if (!m_data.empty()) {
            const char* msg = "fourier_analysis_comp_mask : invalid construction";
            if ( is_odd(m_data.size()) || (m_usize < m_data.back()) ) {
                throw runtime_error(msg);
            }
            for (size_t i = 1; i < m_data.size(); ++i) {
                if (m_data[i] <= m_data[i - 1]) {
                    throw runtime_error(msg);
                }
            }
        }
    }

    fourier_analysis_comp_mask& fourier_analysis_comp_mask::operator&=(fourier_analysis_comp_mask m)
    {
        if_not_compat_then_throw(m);
        this->invert();     // !A
        m.invert();         // !B
        *this |= m;         // !A + !B
        this->invert();     // !(!A + !B)
        return *this;       // De Morgan: A * B = !(!A + !B)
    }

    fourier_analysis_comp_mask& fourier_analysis_comp_mask::operator|=(
        const fourier_analysis_comp_mask& m)
    {
        if_not_compat_then_throw(m);
        for (size_t range = 0; range < m.num_ranges(); ++range) {
            size_t i1 = m.m_data[range * 2];
            size_t i2 = m.m_data[range * 2 + 1];
            this->set_range_safe(i1, i2 - 1); // subtract 1 because i2 is 1-past!
        }
        return *this;
    }

    size_t fourier_analysis_comp_mask::count() const
    {
        size_t count = 0;
        for (size_t range = 0; range < num_ranges(); ++range) {
            const size_t i1 = m_data[range * 2];
            const size_t i2 = m_data[range * 2 + 1];
            for (size_t i = i1; i < i2; ++i) {
                ++count;
            }
        }
        return count;
    }

    std::tuple<diff_t, diff_t, diff_t>
    fourier_analysis_comp_mask::find_max_index(const real_t* data, size_t size) const
    {
        if (m_usize != size) {
            throw runtime_error("fourier_analysis_comp_mask::find_max_index : size error");
        }
        real_t max_value = 0.0;
        diff_t max_index = -1;
        size_t max_range = 0;
        for (size_t range = 0; range < num_ranges(); ++range) {
            const size_t i1 = m_data[range * 2];
            const size_t i2 = m_data[range * 2 + 1];
            for (size_t i = i1; i < i2; ++i) {
                if (max_value < data[i]) {
                    max_value = data[i];
                    max_index = i;
                    max_range = range;
                }
            }
        }
        if (max_index < 0) {
            return std::make_tuple(-1, -1, -1);
        } else {
            diff_t lower = static_cast<diff_t>(m_data[max_range * 2]);
            diff_t upper = static_cast<diff_t>(m_data[max_range * 2 + 1]) - 1;
            return std::make_tuple(max_index, lower, upper);
        }
    }

    std::tuple<size_t, size_t, size_t> fourier_analysis_comp_mask::get_indexes() const
    {
        if (1 == num_ranges()) {
            return std::make_tuple(m_data[0], m_data[1] - 1, m_data[1] - m_data[0]);
        } else if (2 == num_ranges() && m_ufirst == m_data[0] && m_usize == m_data[3]) {
            return std::make_tuple(m_data[2], m_data[1] - 1, m_data[1] + m_data[3] - m_data[2]);
        } else {
            throw runtime_error("fourier_analysis_comp_mask::get_indexes : invalid use");
        }
    }

    void fourier_analysis_comp_mask::invert()
    {
        if (m_data.empty()) {
            set_all();
            return;
        }
        if (m_ufirst == m_data.front()) {
            m_data.erase(m_data.begin());
        } else {
            m_data.insert(m_data.begin(), m_ufirst);
        }
        if (m_usize == m_data.back()) {
            m_data.pop_back();
        } else {
            m_data.push_back(m_usize);
        }
    }

    bool fourier_analysis_comp_mask::overlaps(size_t left, size_t right) const
    {
        size_t left_index = get_index(left);
        size_t right_index = get_index(right);
        if (is_odd(left_index) || is_odd(right_index)) { // if either are inside a range
            return true;
        }
        return left_index != right_index; // true if left and right straddle one or more ranges
    }

    void fourier_analysis_comp_mask::set_range(diff_t left, diff_t right)
    {
        if (right < left) {
            throw runtime_error("fourier_analysis_comp_mask::set_range : right < left");
        }
        size_t uleft = 0;
        size_t uright = 0;
        if (Stop == m_mode) {
            uleft  = static_cast<size_t>(std::max(m_first, left));
            uright = static_cast<size_t>(std::min(right, m_last));
        } else {
            if (m_last < right - left) {
                throw runtime_error("fourier_analysis_comp_mask::set_range : size < range");
            }
            if (left < m_first) {
                left += m_size;
                if (left < m_first) {
                    throw runtime_error("fourier_analysis_comp_mask::set_range : left out of range");
                }
            }
            if (m_last < right) {
                right -= m_size;
                if (m_last < right) {
                    throw runtime_error("fourier_analysis_comp_mask::set_range : right out of range");
                }
            }
            uleft  = static_cast<size_t>(left);
            uright = static_cast<size_t>(right);
        }
        if (uright < uleft) {
            set_range_safe(m_ufirst, uright);
            set_range_safe(uleft, m_ulast);
        } else {
            set_range_safe(uleft, uright);
        }
    }

    real_t fourier_analysis_comp_mask::sum(const real_t* data, size_t size) const
    {
        if (m_usize != size) {
            throw runtime_error("fourier_analysis_comp_mask::sum : size error");
        }
        real_t mag = 0.0;
        for (size_t range = 0; range < num_ranges(); ++range) {
            const size_t i1 = m_data[range * 2];
            const size_t i2 = m_data[range * 2 + 1];
            for (size_t i = i1; i < i2; ++i) {
                mag += data[i];
            }
        }
        return mag;
    }

    void fourier_analysis_comp_mask::unset_ranges(const fourier_analysis_comp_mask& m)
    {
        if (&m == this) {
            this->clear();
            return;
        }
        if_not_compat_then_throw(m);
        this->invert();
        *this |= m;
        this->invert();
    }

    size_t fourier_analysis_comp_mask::get_index(size_t value) const
    {
        size_t index = 0;
        while (index < m_data.size()) {
            if (value < m_data[index]) {
                break;
            }
            ++index;
        }
        return index;
    }

    void fourier_analysis_comp_mask::if_not_compat_then_throw(const fourier_analysis_comp_mask& m)
    {
        if ((m.m_mode != this->m_mode) || (m.m_size != this->m_size)) {
            throw runtime_error("fourier_analysis_comp_mask : mask is incompatible");
        }
    }

    void fourier_analysis_comp_mask::set_range_safe(size_t left, size_t right)
    {
        right += 1; // right is now 1-past
        if (m_data.empty() || m_data.back() < left) {
            m_data.push_back(left);
            m_data.push_back(right);
            return;
        }
        size_t left_index = get_index(left);
        size_t right_index = get_index(right);
        // left and left_index adjustments
        if (is_even(left_index)) { // even: left is not in a range
            // Explain this...
            if (0 < left_index && left == m_data[left_index - 1]) {
                left_index -= 2;
                left = m_data[left_index];
            }
        } else { // odd: left is in a range
            // Explain this...
            left_index -= 1;
            left = m_data[left_index];
        }
        // right and right_index adjustments
        if (is_odd(right_index)) { // odd: right is in a range
            // Explain this...
            right = m_data[right_index];
            right_index += 1;
        }
        // Update m_data
        m_data.erase(m_data.begin() + left_index, m_data.begin() + right_index);
        m_data.insert(m_data.begin() + left_index, {left, right});
    }

} // namespace genalyzer_impl