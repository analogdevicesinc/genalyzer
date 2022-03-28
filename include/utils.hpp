#ifndef DCANALYSIS_IMPL_UTILS_HPP
#define DCANALYSIS_IMPL_UTILS_HPP

#include "constants.hpp"
#include "enums.hpp"
#include "exceptions.hpp"
#include "type_aliases.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <type_traits>

namespace dcanalysis_impl {

    inline real_t bounded_db10(real_t msq_value)
    {
        return 10 * std::log10(std::clamp(msq_value, k_abs_min_msq, k_abs_max_msq));
    }

    inline real_t bounded_db20(real_t rms_value)
    {
        return 20 * std::log10(std::clamp(rms_value, k_abs_min_rms, k_abs_max_rms));
    }

    inline real_t bounded_db(const cplx_t& cplx_value)
    {
        return bounded_db10(std::norm(cplx_value));
    }

    template<typename T>
    bool is_even(T n)
    {
        return 0 == n % 2;
    }

    template<typename T>
    bool is_odd(T n)
    {
        return 1 == n % 2;
    }

    template<typename T>
    bool is_pow2(T n)
    {
        return (n < 1) ? false : (0 == (n & (n - 1)));
    }

    template<typename E>
    int to_int(E e)
    {
        static_assert(std::is_enum_v<E>, "Requires enum type");
        return static_cast<int>(e);
    }

    template<typename T>
    void assert_eq(const char* trace, const char* name1, T val1, const char* name2, T val2)
    {
        if (val1 != val2) {
            throw runtime_error(str_t(trace) + name1 + " != " + name2);
        }
    }

    template<typename T>
    void assert_gt0(const char* trace, const char* name, T val)
    {
        if (!(static_cast<T>(0) < val)) {
            throw runtime_error(str_t(trace) + name + " <= 0");
        }
    }

    template<typename T>
    void assert_ptr_not_null(const char* trace, const char* name, const T* p)
    {
        if (nullptr == p) {
            throw runtime_error(str_t(trace) + "pointer to " + name + " is NULL");
        }
    }

    template<typename T>
    void check_array(
        const char* trace, const char* name, const T* p, size_t size, bool interleaved = false)
    {
        assert_ptr_not_null(trace, name, p);
        str_t n = str_t(name) + " size";
        assert_gt0(trace, n.c_str(), size);
        if (interleaved && is_odd(size)) {
            throw runtime_error(str_t(trace) + " must be even if interleaved");
        }
    }

    template<typename T1, typename T2>
    void check_array_pair(
        const char* trace,
        const char* name1,
        const T1* p1,
        size_t size1,
        const char* name2,
        const T2* p2,
        size_t size2,
        bool interleaved = false
        )
    {
        check_array(trace, name1, p1, size1);
        check_array(trace, name2, p2, size2);
        str_t n1 = str_t(name1) + " size";
        str_t n2 = str_t(name2) + " size";
        assert_eq(trace, n1.c_str(), size1, n2.c_str(), size2);
        if (interleaved && is_odd(size1)) {
            throw runtime_error(str_t(trace) + " must be even if interleaved");
        }
    }

    inline void check_code_width(const char* trace, int n)
    {
        if (n < k_abs_min_code_width || k_abs_max_code_width < n) {
            throw runtime_error(str_t(trace) + "resolution outside absolute code width limits : ["
                + std::to_string(k_abs_min_code_width) + ", "
                + std::to_string(k_abs_max_code_width) + "]");
        }
    }

    template<typename T>
    T check_type_absolute_value(const char* trace, int64_t n)
    {
        static_assert(sizeof(T) <= sizeof(int64_t), "T is bigger than int64_t");
        static_assert(std::is_signed_v<T>, "T must be signed");
        constexpr int64_t min = static_cast<int64_t>(std::numeric_limits<T>::min());
        constexpr int64_t max = static_cast<int64_t>(std::numeric_limits<T>::max());
        if (n < min || max < n) {
            throw runtime_error(str_t(trace) + "value exceeds type limits");
        }
        return static_cast<T>(n);
    }

    template<typename T>
    std::pair<T, T> resolution_to_minmax(int n, CodeFormat format)
    {
        const char* trace = "resolution_to_minmax : ";
        check_code_width(trace, n);                                 // for example, let n = 8
        int64_t min64 = -(static_cast<int64_t>(1) << (n - 1));      // min = -128
        int64_t max64 = -1 - min64;                                 // max = +127
        if (CodeFormat::OffsetBinary == format) {
            max64 -= min64;                                         // max = 255
            min64 = 0;                                              // min = 0
        }
        T min = check_type_absolute_value<T>(trace, min64);
        T max = check_type_absolute_value<T>(trace, max64);
        return std::make_pair(min, max);
    }

    str_t to_string(real_t n, FPFormat fmt = FPFormat::Auto, int max_prec = -1);

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_UTILS_HPP