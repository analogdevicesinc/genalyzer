/*
File        : $HeadURL$
Originator  : pderouni
Revision    : $Revision$
Last Commit : $Date$
Last Editor : $Author$
*/

#ifndef ICD_ANALYSIS_SPARSE_ARRAY_MASK_HPP
#define ICD_ANALYSIS_SPARSE_ARRAY_MASK_HPP

#include "analysis.hpp"
#include "types.hpp"

namespace analysis {

class ICD_ANALYSIS_DECL sparse_array_mask {
public:
    using range_t = std::pair<diff_t, diff_t>; // STL-like: [ a, b )
    using container_t = std::vector<range_t>;

public:
    sparse_array_mask(diff_t size = 1);
    sparse_array_mask(const sparse_array_mask& m);
    sparse_array_mask(sparse_array_mask&& m);

    ~sparse_array_mask() = default;

public:
    sparse_array_mask& operator=(const sparse_array_mask& m);
    sparse_array_mask& operator=(sparse_array_mask&& m);
    sparse_array_mask& operator|=(const sparse_array_mask& m);

public:
    diff_t array_size() const;
    diff_t num_ranges() const;
    diff_t num_open_ranges() const;

public:
    const container_t& all_ranges() const;
    container_t all_open_ranges() const;

    range_t get_range(diff_t index) const;
    range_t get_open_range(diff_t index) const;

    void clear();
    void set_all();
    void set_range(diff_t left, diff_t right);
    void set_range(range_t range);
    void unset_range(diff_t left, diff_t right);
    void unset_range(range_t range);

public: // added with FFT analysis in mind:
    bool equals(const sparse_array_mask& m) const;
    bool in_open_range(real_t center) const;
    bool is_nonoverlapping(size_t left, size_t right) const;
    void set_range2(size_t left, size_t right);
    std::pair<real_t, size_t> sum_ranges(const real_vector& data) const;
    std::pair<real_t, size_t> sum_open_ranges(const real_vector& data) const;

public:
    std::string debug_string() const;

private:
    diff_t normalize_index(diff_t index) const;
    diff_t first_left_index() const;
    diff_t last_right_index() const;

private:
    /**
         * The "location" of an index in a sparse_array_mask is defined in
         * terms of the range it lies in or the ranges it lies between.
         * (  n, n  ) The index lies in Range [n].
         * (  n, n+1) The index lies between Range [n] and Range [n+1].
         * ( -1, 0  ) The index lies before Range [0] (before the first range).
         * (N-1, N  ) The index lies after Range [N-1] (after the last range).
         */
    using location_t = std::pair<diff_t, diff_t>;

    location_t locate_index(diff_t index) const;

private:
    diff_t m_array_size;
    container_t m_ranges;

}; // class array_mask

inline bool operator==(const sparse_array_mask& lhs,
    const sparse_array_mask& rhs)
{
    return lhs.equals(rhs);
}

inline sparse_array_mask operator|(const sparse_array_mask& left,
    const sparse_array_mask& right)
{
    sparse_array_mask mask(left);
    mask |= right;
    return mask;
}

} // namespace analysis

#endif // ICD_ANALYSIS_SPARSE_ARRAY_MASK_HPP
