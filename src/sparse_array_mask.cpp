/*
File        : $HeadURL$
Originator  : pderouni
Revision    : $Revision$
Last Commit : $Date$
Last Editor : $Author$
*/

#include "sparse_array_mask.hpp"
#include <algorithm>

namespace analysis {

sparse_array_mask::sparse_array_mask(diff_t size)
    : m_array_size{ size }
    , m_ranges{}
{
}

sparse_array_mask::sparse_array_mask(const sparse_array_mask& m)
    : m_array_size{ m.m_array_size }
    , m_ranges(m.m_ranges)
{
}

sparse_array_mask::sparse_array_mask(sparse_array_mask&& m)
    : m_array_size{ m.m_array_size }
    , m_ranges(std::move(m.m_ranges))
{
}

sparse_array_mask& sparse_array_mask::operator=(const sparse_array_mask& m)
{
    if (&m != this) {
        sparse_array_mask the_copy(m);
        std::swap(the_copy, *this);
    }
    return *this;
}

sparse_array_mask& sparse_array_mask::operator=(sparse_array_mask&& m)
{
    std::swap(m_array_size, m.m_array_size);
    std::swap(m_ranges, m.m_ranges);
    return *this;
}

sparse_array_mask& sparse_array_mask::operator|=(const sparse_array_mask& m)
{
    if (&m != this) {
        for (const range_t& r : m.all_ranges()) {
            set_range(r);
        }
    }
    return *this;
}

diff_t sparse_array_mask::array_size() const
{
    return m_array_size;
}

diff_t sparse_array_mask::num_ranges() const
{
    return static_cast<diff_t>(m_ranges.size());
}

diff_t sparse_array_mask::num_open_ranges() const
{
    if (m_ranges.empty()) {
        return 1;
    }
    auto n = num_ranges() - 1;
    if (first_left_index() != 0) {
        n += 1;
    }
    if (last_right_index() != m_array_size) {
        n += 1;
    }
    return n;
}

const sparse_array_mask::container_t& sparse_array_mask::all_ranges() const
{
    return m_ranges;
}

sparse_array_mask::container_t sparse_array_mask::all_open_ranges() const
{
    container_t open_ranges;
    if (m_ranges.empty()) {
        open_ranges.emplace_back(0, m_array_size);
        return open_ranges;
    }
    if (first_left_index() != 0) {
        open_ranges.emplace_back(0, m_ranges.front().first);
    }
    // there must be >= 2 ranges to go into the following loop
    for (diff_t i = 1; i < num_ranges(); ++i) {
        open_ranges.emplace_back(m_ranges.data()[i - 1].second,
            m_ranges.data()[i].first);
    }
    if (last_right_index() != m_array_size) {
        open_ranges.emplace_back(m_ranges.back().second, m_array_size);
    }
    return open_ranges;
}

sparse_array_mask::range_t
sparse_array_mask::get_range(diff_t index) const
{
    index = normalize_index(index);
    const auto loc = locate_index(index);
    if (loc.first == loc.second) {
        return m_ranges.data()[loc.first];
    } else {
        return std::make_pair(index, index);
    }
}

sparse_array_mask::range_t
sparse_array_mask::get_open_range(diff_t index) const
{
    if (m_ranges.empty()) {
        return std::make_pair(0, m_array_size);
    }
    index = normalize_index(index);
    const auto loc = locate_index(index);
    if (loc.first == loc.second) {
        return std::make_pair(index, index);
    } else if (-1 == loc.first) {
        return std::make_pair(0, m_ranges.front().first);
    } else if (num_ranges() == loc.second) {
        return std::make_pair(m_ranges.back().second, m_array_size);
    } else {
        return std::make_pair(m_ranges.data()[loc.first].second,
            m_ranges.data()[loc.second].first);
    }
}

void sparse_array_mask::clear()
{
    m_ranges.clear();
}

void sparse_array_mask::set_all()
{
    m_ranges.clear();
    m_ranges.emplace_back(0, m_array_size);
}

// note: left and right are STL-like range: [ left, right )
void sparse_array_mask::set_range(diff_t left, diff_t right)
{
    left = normalize_index(left);
    if (m_array_size != right) {
        right = normalize_index(right);
    }
    if (left == right) {
        return;
    }
    if (left > right) {
        set_range(left, m_array_size);
        set_range(0, right);
        return;
    }
    // at this point, left < right
    auto lloc = locate_index(left);
    auto rloc = locate_index(right);
    /* If the left index abuts the right end of the range on its left,
         * adjust lloc as though it (the left index) is in fact in the range on
         * its left.  There is no need to do something equivalent for the right
         * index because the right index itself is not part of the range being
         * set (the pair of range indexes are STL-like: inclusive left,
         * exclusive right).  If the range being set abuts the range on its
         * right, the location of right index will already indicate that by
         * being found in the range on the right (it will equal the first index
         * of the range on the right).  In other words, if the range being set
         * abuts the range to its right, the right index of the new range will
         * equal the left index of the right range.  In the example below, the
         * right index of the new range equals the left index of the existing
         * range.
         *   Existing Range [6,9):       ***
         *        New Range [3,6):    ***
         *                  Index: 0123456789
         */
    if (lloc.first != lloc.second && -1 < lloc.first) {
        if (m_ranges.data()[lloc.first].second == left) {
            lloc.second = lloc.first;
        }
    }
    // now the actual mask setting - four cases: {left,right} * {in,out}
    if (lloc.first == lloc.second && // left in
        rloc.first == rloc.second) { // right in
        if (lloc.first != rloc.first) {
            // nothing to do when L & R are in the same range
            //    *****      *****    ******
            //        |                 |
            //   (0,0)L                 R(2,2)
            m_ranges.data()[lloc.first].second = m_ranges.data()[rloc.first].second;
            m_ranges.erase(m_ranges.cbegin() + lloc.first + 1,
                m_ranges.cbegin() + rloc.first + 1);
        }
    } else if (lloc.first == lloc.second && // left in
        rloc.first == rloc.second - 1) { // right out
        //     *******       ****
        //        |                   |
        //   (0,0)L                   R(1,2)
        m_ranges.data()[lloc.first].second = right;
        m_ranges.erase(m_ranges.cbegin() + lloc.first + 1,
            m_ranges.cbegin() + rloc.second);
    } else if (lloc.first == lloc.second - 1 && // left out
        rloc.first == rloc.second) { // right in
        //     ******      ****    *****
        //               |           |
        //          (0,1)L           R(2,2)
        m_ranges.data()[rloc.first].first = left;
        m_ranges.erase(m_ranges.cbegin() + lloc.second,
            m_ranges.cbegin() + rloc.first);
    } else if (lloc.first == lloc.second - 1 && // left out
        rloc.first == rloc.second - 1) { // right out
        //    ***             ***
        //             |  |
        //        (0,1)L  R(0,1)
        //
        //    ***       ***   ****     ***
        //           |              |
        //      (0,1)L              R(2,3)
        m_ranges.erase(m_ranges.cbegin() + lloc.second,
            m_ranges.cbegin() + rloc.second);
        m_ranges.emplace(m_ranges.cbegin() + lloc.second, left, right);
    } else {
        // should never happen!
    }
}

void sparse_array_mask::set_range(sparse_array_mask::range_t range)
{
    set_range(range.first, range.second);
}

// note: left and right are STL-like range: [ left, right )
void sparse_array_mask::unset_range(diff_t left, diff_t right)
{
    left = normalize_index(left);
    if (m_array_size != right) {
        right = normalize_index(right);
    }
    if (left == right) {
        return;
    }
    if (left > right) {
        unset_range(left, m_array_size);
        unset_range(0, right);
        return;
    }
    // at this point, left < right
    auto lloc = locate_index(left);
    auto rloc = locate_index(right);
    // If Left (Right) equals the first index in a range, adjust the
    // location to make it appear as though Left (Right) is between the
    // range it is in and the one prior.
    if (lloc.first == lloc.second) {
        if (m_ranges.data()[lloc.first].first == left) {
            lloc.first -= 1;
        }
    }
    if (rloc.first == rloc.second) {
        if (m_ranges.data()[rloc.first].first == right) {
            rloc.first -= 1;
        }
    }
    // now the actual mask unsetting - four cases: {left,right} * {in,out}
    if (lloc.first == lloc.second - 1 && // left out
        rloc.first == rloc.second - 1) { // right out
        if (lloc.first != rloc.first) {
            // nothing to do when L & R are in the same open range
            //  *****   ***    ***       ***
            //        |               |
            //   (0,1)L               R(2,3)
            m_ranges.erase(m_ranges.cbegin() + lloc.second,
                m_ranges.cbegin() + rloc.second);
        }
    } else if (lloc.first == lloc.second - 1 && // left out
        rloc.first == rloc.second) { // right in
        //   ***      *******    ***    *****
        //        |                        |
        //   (0,1)L                        R(3,3)
        m_ranges.data()[rloc.first].first = right;
        m_ranges.erase(m_ranges.cbegin() + lloc.second,
            m_ranges.cbegin() + rloc.first);
    } else if (lloc.first == lloc.second && // left in
        rloc.first == rloc.second - 1) { // right out
        //   *******      *******    ***    *****
        //        |                        |
        //   (0,0)L                        R(2,3)
        m_ranges.data()[lloc.first].second = left;
        m_ranges.erase(m_ranges.cbegin() + lloc.first + 1,
            m_ranges.cbegin() + rloc.second);
    } else if (lloc.first == lloc.second && // left in
        rloc.first == rloc.second) { // right in
        //   ****************
        //        |       |
        //   (0,0)L       R(0,0)
        //
        //   *******      *******    ************
        //        |                        |
        //   (0,0)L                        R(2,2)
        const auto last = m_ranges.data()[rloc.first].second;
        m_ranges.data()[lloc.first].second = left;
        m_ranges.erase(m_ranges.cbegin() + lloc.first + 1,
            m_ranges.cbegin() + rloc.first + 1);
        set_range(right, last);
    } else {
        // should never happen!
    }
}

void sparse_array_mask::unset_range(sparse_array_mask::range_t range)
{
    unset_range(range.first, range.second);
}

bool sparse_array_mask::equals(const sparse_array_mask& m) const
{
    return (m_array_size == m.m_array_size) && (m_ranges == m.m_ranges);
}

bool sparse_array_mask::in_open_range(real_t center) const
{
    auto left = static_cast<diff_t>(std::floor(center));
    auto right = static_cast<diff_t>(std::ceil(center));
    if (left == right) {
        range_t r = get_range(left);
        return r.first == r.second;
    } else {
        if (m_array_size == right) {
            right -= m_array_size;
        }
        range_t rl = get_range(left);
        range_t rr = get_range(right);
        return (rl.first == rl.second) && (rr.first == rr.second);
    }
}

bool sparse_array_mask::is_nonoverlapping(size_t left, size_t right) const
{
    sparse_array_mask m1(m_array_size);
    m1.set_range2(left, right);
    sparse_array_mask m2(m1);
    for (const range_t& r : m_ranges) {
        m1.unset_range(r.first, r.second);
    }
    return m1 == m2;
}

void sparse_array_mask::set_range2(size_t left, size_t right)
{
    set_range(static_cast<diff_t>(left), static_cast<diff_t>(right));
}

std::pair<real_t, size_t>
sparse_array_mask::sum_ranges(const real_vector& data) const
{
    real_t sum = 0.0;
    size_t num = 0;
    for (const range_t& r : m_ranges) {
        auto i1 = static_cast<size_t>(r.first);
        auto i2 = static_cast<size_t>(r.second);
        num += i2 - i1;
        for (size_t i = i1; i < i2; ++i) {
            sum += data[i];
        }
    }
    return std::make_pair(sum, num);
}

std::pair<real_t, size_t>
sparse_array_mask::sum_open_ranges(const real_vector& data) const
{
    real_t sum = 0.0;
    size_t num = 0;
    for (const range_t& r : all_open_ranges()) {
        auto i1 = static_cast<size_t>(r.first);
        auto i2 = static_cast<size_t>(r.second);
        num += i2 - i1;
        for (size_t i = i1; i < i2; ++i) {
            sum += data[i];
        }
    }
    return std::make_pair(sum, num);
}

std::string sparse_array_mask::debug_string() const
{
    std::ostringstream oss;
    diff_t i = 0;
    const diff_t max_limit = 128;
    const diff_t limit = std::min(m_array_size, max_limit);
    for (const auto& r : m_ranges) {
        for (; i < r.first && i < limit; ++i) {
            oss << ' ';
        }
        for (; i < r.second && i < limit; ++i) {
            oss << '*';
        }
    }
    oss << '\n';
    for (diff_t j = 0; j < limit; ++j) {
        oss << (j % 10);
    }
    oss << '\n';
    for (diff_t j = 0; j < limit; ++j) {
        if (0 == j % 10) {
            oss << ((j / 10) % 10);
        } else {
            oss << ' ';
        }
    }
    oss << '\n';
    oss << num_ranges() << " ranges full: ";
    for (const auto& r : all_ranges()) {
        oss << '[' << r.first << ',' << r.second << "), ";
    }
    oss << '\n';
    oss << num_open_ranges() << " ranges open: ";
    for (const auto& r : all_open_ranges()) {
        oss << '[' << r.first << ',' << r.second << "), ";
    }
    oss << '\n';
    return oss.str();
}

diff_t sparse_array_mask::normalize_index(diff_t index) const
{
    if (0 > index) {
        index += m_array_size;
    }
    if (0 > index || m_array_size <= index) {
        throw base::exception("Index out of range.");
    }
    return index;
}

diff_t sparse_array_mask::first_left_index() const
{
    return m_ranges.empty() ? m_array_size : m_ranges.front().first;
}

diff_t sparse_array_mask::last_right_index() const
{
    return m_ranges.empty() ? m_array_size : m_ranges.back().second;
}

sparse_array_mask::location_t
sparse_array_mask::locate_index(diff_t index) const
{
    diff_t range_index = 0;
    for (const auto& r : m_ranges) {
        if (r.first > index) { // index between ranges r-1 and r
            break;
        } else if (r.second > index) { // index in range r
            return std::make_pair(range_index, range_index);
        }
        ++range_index;
    }
    return std::make_pair(range_index - 1, range_index);
}

} // namespace analysis
