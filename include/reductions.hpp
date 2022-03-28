#ifndef DCANALYSIS_IMPL_REDUCTIONS_HPP
#define DCANALYSIS_IMPL_REDUCTIONS_HPP

#include "type_aliases.hpp"

#include <map>

namespace dcanalysis_impl {

    struct std_reduce_t
    {
        std_reduce_t(size_t size)
            : min {0.0},
              max {0.0},
              sum {0.0},
              sumsq {0.0},
              min_index {size},
              max_index {size}
        {}
        real_t min;
        real_t max;
        real_t sum;
        real_t sumsq;
        size_t min_index;
        size_t max_index;
    };

    template<typename T>
    std_reduce_t std_reduce(
        const T* data,          // pointer to array
        const size_t size,      // array size
        const size_t i1,        // index of first element
        const size_t i2         // one past the last element
        )
    {
        std_reduce_t r (size);
        if (i1 < i2 && i2 <= size) {
            r.min = static_cast<real_t>(data[i1]);
            r.max = static_cast<real_t>(data[i1]);
            r.min_index = i1;
            r.max_index = i1;
            for (size_t i = i1 + 1; i < i2; ++i) {
                const real_t x = static_cast<real_t>(data[i]);
                if (x < r.min) {
                    r.min = x;
                    r.min_index = i;
                }
                if (r.max < x) {
                    r.max = x;
                    r.max_index = i;
                }
                r.sum += x;
                r.sumsq += x * x;
            }
        }
        return r;
    }

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_REDUCTIONS_HPP