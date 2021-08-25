/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/array_math.cpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#include "array_math.hpp"
#include "checks.hpp"
#include <algorithm>

namespace analysis {

namespace {

    void __c2r(const cplx_t* in_data, size_t in_size,
        real_t* out_data, size_t out_size,
        real_t (*func)(const cplx_t&))
    {
        check_array(in_data, in_size, "input array");
        check_array(out_data, out_size, "output array");
        assert_eq(in_size, "input array size",
            out_size, "output array size");
        std::transform(in_data, in_data + in_size, out_data, func);
    }

    void __r2c(const real_t* in1_data, size_t in1_size,
        const real_t* in2_data, size_t in2_size,
        cplx_t* out_data, size_t out_size,
        cplx_t (*func)(const real_t&, const real_t&))
    {
        check_array_pair(in1_data, in1_size, "input array 1",
            in2_data, in2_size, "input array 2");
        check_array(out_data, out_size, "output array");
        assert_eq(in1_size, "input array size",
            out_size, "output array size");
        std::transform(in1_data, in1_data + in1_size, in2_data,
            out_data, func);
    }

    void __r2r(const real_t* in_data, size_t in_size,
        real_t* out_data, size_t out_size,
        real_t (*func)(const real_t&))
    {
        check_array(in_data, in_size, "input array");
        check_array(out_data, out_size, "output array");
        assert_eq(in_size, "input array size",
            out_size, "output array size");
        std::transform(in_data, in_data + in_size, out_data, func);
    }

} // namespace anonymous

void abs(const cplx_t* in_data, size_t in_size,
    real_t* out_data, size_t out_size)
{
    __c2r(in_data, in_size, out_data, out_size, std::abs<real_t>);
}

void angle(const cplx_t* in_data, size_t in_size,
    real_t* out_data, size_t out_size)
{
    __c2r(in_data, in_size, out_data, out_size, std::arg<real_t>);
}

void complex(const real_t* re_data, size_t re_size,
    const real_t* im_data, size_t im_size,
    cplx_t* out_data, size_t out_size)
{
    __r2c(re_data, re_size, im_data, im_size, out_data, out_size,
        [](const real_t& r, const real_t& i) { return cplx_t(r, i); });
}

void db(const cplx_t* in_data, size_t in_size,
    real_t* out_data, size_t out_size)
{
    __c2r(in_data, in_size, out_data, out_size,
        [](const cplx_t& c) { return 10 * std::log10(std::norm(c)); });
}

void db10(const real_t* in_data, size_t in_size,
    real_t* out_data, size_t out_size)
{
    __r2r(in_data, in_size, out_data, out_size,
        [](const real_t& x) { return 10 * std::log10(x); });
}

void db20(const real_t* in_data, size_t in_size,
    real_t* out_data, size_t out_size)
{
    __r2r(in_data, in_size, out_data, out_size,
        [](const real_t& x) { return 20 * std::log10(x); });
}

void imag(const cplx_t* in_data, size_t in_size,
    real_t* out_data, size_t out_size)
{
    __c2r(in_data, in_size, out_data, out_size, std::imag<real_t>);
}

void norm(const cplx_t* in_data, size_t in_size,
    real_t* out_data, size_t out_size)
{
    __c2r(in_data, in_size, out_data, out_size, std::norm<real_t>);
}

void polar(const real_t* m_data, size_t m_size,
    const real_t* p_data, size_t p_size,
    cplx_t* out_data, size_t out_size)
{
    __r2c(m_data, m_size, p_data, p_size, out_data, out_size,
        std::polar<real_t>);
}

void real(const cplx_t* in_data, size_t in_size,
    real_t* out_data, size_t out_size)
{
    __c2r(in_data, in_size, out_data, out_size, std::real<real_t>);
}

} // namespace analysis
