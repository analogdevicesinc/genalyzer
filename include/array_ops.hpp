#ifndef DCANALYSIS_IMPL_ARRAY_OPS_HPP
#define DCANALYSIS_IMPL_ARRAY_OPS_HPP

#include "type_aliases.hpp"

namespace dcanalysis_impl {

    void abs(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size);

    void angle(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size);

    void db(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size);

    void db10(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size);

    void db20(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size);

    void norm(const real_t* in_data, size_t in_size, real_t* out_data, size_t out_size);

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_ARRAY_OPS_HPP