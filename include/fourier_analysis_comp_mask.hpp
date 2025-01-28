// Copyright (C) 2024-2025 Analog Devices, Inc.
//
// SPDX short identifier: ADIBSD OR GPL-2.0-or-later
#ifndef GENALYZER_IMPL_FOURIER_ANALYSIS_COMP_MASK_HPP
#define GENALYZER_IMPL_FOURIER_ANALYSIS_COMP_MASK_HPP

#include "type_aliases.hpp"
#include <tuple>

namespace genalyzer_impl {

// Ranges are set as [first, last], i.e., inclusive-inclusive,
// but are stored [first, last+1) like STL, i.e., inclusive-exclusive
class fourier_analysis_comp_mask {
public:
	enum BoundaryMode { Stop,
		Wrap };

public:
	fourier_analysis_comp_mask(bool cplx, size_t array_size) :
			m_mode{ cplx ? Wrap : Stop }, m_ufirst{ 0 }, m_ulast{ array_size - 1 }, m_usize{ array_size }, m_first{ 0 }, m_last{ static_cast<diff_t>(array_size - 1) }, m_size{ static_cast<diff_t>(array_size) }, m_data{} {
	}

	fourier_analysis_comp_mask(bool cplx, size_t array_size,
			const std::vector<size_t> &init);

	fourier_analysis_comp_mask(const fourier_analysis_comp_mask &m) :
			m_mode{ m.m_mode }, m_ufirst{ m.m_ufirst }, m_ulast{ m.m_ulast }, m_usize{ m.m_usize }, m_first{ m.m_first }, m_last{ m.m_last }, m_size{ m.m_size }, m_data(m.m_data) {
	}

	fourier_analysis_comp_mask(fourier_analysis_comp_mask &&m) :
			m_mode{ m.m_mode }, m_ufirst{ m.m_ufirst }, m_ulast{ m.m_ulast }, m_usize{ m.m_usize }, m_first{ m.m_first }, m_last{ m.m_last }, m_size{ m.m_size }, m_data(std::move(m.m_data)) {
	}

	~fourier_analysis_comp_mask() = default;

public:
	fourier_analysis_comp_mask &
	operator=(const fourier_analysis_comp_mask &m) {
		if (&m != this) {
			fourier_analysis_comp_mask the_copy(m);
			std::swap(the_copy, *this);
		}
		return *this;
	}

	fourier_analysis_comp_mask &operator=(fourier_analysis_comp_mask &&m) {
		std::swap(this->m_mode, m.m_mode);
		std::swap(this->m_ufirst, m.m_ufirst);
		std::swap(this->m_ulast, m.m_ulast);
		std::swap(this->m_usize, m.m_usize);
		std::swap(this->m_first, m.m_first);
		std::swap(this->m_last, m.m_last);
		std::swap(this->m_size, m.m_size);
		std::swap(this->m_data, m.m_data);
		return *this;
	}

	fourier_analysis_comp_mask &operator&=(fourier_analysis_comp_mask m);

	fourier_analysis_comp_mask &
	operator|=(const fourier_analysis_comp_mask &m);

public:
	void clear() {
		m_data.clear();
	}

	// Returns the number of elements in the ranges
	size_t count() const;

	real_t count_r() const {
		return static_cast<real_t>(count());
	}

	const std::vector<size_t> &data() const {
		return m_data;
	}

	bool equals(const fourier_analysis_comp_mask &that) const {
		return (this->m_usize == that.m_usize) &&
				(this->m_data == that.m_data);
	}

	// <0> = index of max element (-1 if not found)
	// <1> = last contiguous free index to the left
	// <2> = last contiguous free index to the right
	std::tuple<diff_t, diff_t, diff_t> find_max_index(const real_t *data,
			size_t size) const;

	// Returns Index1, Index2, Number of Bins
	std::tuple<size_t, size_t, size_t> get_indexes() const;

	void invert();

	BoundaryMode mode() const {
		return m_mode;
	}

	size_t num_ranges() const {
		return m_data.size() / 2;
	}

	// Returns true if [left, right] overlaps any ranges
	bool overlaps(size_t left, size_t right) const; // inclusive-inclusive

	void set(std::vector<size_t> init) {
		fourier_analysis_comp_mask new_mask((Wrap == this->m_mode),
				this->m_usize, init);
		std::swap(new_mask, *this);
	}

	void set_all() {
		m_data = { 0, m_usize };
	}

	void set_range(diff_t left, diff_t right); // inclusive-inclusive

	size_t size() const {
		return m_usize;
	}

	real_t root_sum(const real_t *data, size_t size) const {
		return std::sqrt(sum(data, size));
	}

	// Returns the sum of the data in the ranges
	real_t sum(const real_t *data, size_t size) const;

	void unset_ranges(const fourier_analysis_comp_mask &m);

private:
	// Returns the index of the first element greater than 'value'
	size_t get_index(size_t value) const;

	void if_not_compat_then_throw(const fourier_analysis_comp_mask &m);

	// Document the pre-conditions that make this 'safe'
	void set_range_safe(size_t left, size_t right); // inclusive-inclusive

private:
	BoundaryMode m_mode;
	size_t m_ufirst;
	size_t m_ulast;
	size_t m_usize;
	diff_t m_first;
	diff_t m_last;
	diff_t m_size;
	std::vector<size_t> m_data;

}; // class fourier_analysis_comp_mask

inline bool operator==(const fourier_analysis_comp_mask &a,
		const fourier_analysis_comp_mask &b) {
	return a.equals(b);
}

inline bool operator!=(const fourier_analysis_comp_mask &a,
		const fourier_analysis_comp_mask &b) {
	return !a.equals(b);
}

} // namespace genalyzer_impl

#endif // GENALYZER_IMPL_FOURIER_ANALYSIS_COMP_MASK_HPP