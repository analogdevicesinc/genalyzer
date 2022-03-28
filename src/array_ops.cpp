#include "array_ops.hpp"

#include "utils.hpp"

#include <algorithm>
#include <cmath>

namespace dcanalysis_impl {

    namespace {

        void c2r(
            const real_t* in_data,
            size_t in_size,
            real_t* out_data,
            size_t out_size,
            real_t(*func)(const cplx_t&)
            )
        {
            assert_eq("", "input array size", in_size, "2 * output array size", 2 * out_size);
            const cplx_t* cin_data = reinterpret_cast<const cplx_t*>(in_data);
            size_t cin_size = in_size / 2;
            check_array_pair("", "input array", cin_data, cin_size, "output array", out_data, out_size);
            std::transform(cin_data, cin_data + cin_size, out_data, func);
        }

        void r2r(
            const real_t* in_data,
            size_t in_size,
            real_t* out_data,
            size_t out_size,
            real_t(*func)(real_t)
            )
        {
            check_array_pair("", "input array", in_data, in_size, "output array", out_data, out_size);
            std::transform(in_data, in_data + in_size, out_data, func);
        }

    } // namespace anonymous

    void abs(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size)
    {
        c2r(in_data, in_size, out_data, out_size, std::abs<real_t>);
    }

    void angle(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size)
    {
        c2r(in_data, in_size, out_data, out_size, std::arg<real_t>);
    }

    void db(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size)
    {
        c2r(in_data, in_size, out_data, out_size, bounded_db);
    }

    void db10(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size)
    {
        r2r(in_data, in_size, out_data, out_size, bounded_db10);
    }

    void db20(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size)
    {
        r2r(in_data, in_size, out_data, out_size, bounded_db20);
    }

    void norm(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size)
    {
        c2r(in_data, in_size, out_data, out_size, std::norm<real_t>);
    }

} // namespace dcanalysis_impl