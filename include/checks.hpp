/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/checks.hpp $
Originator  : pderouni
Revision    : $Revision: 12382 $
Last Commit : $Date: 2020-04-06 11:21:16 -0400 (Mon, 06 Apr 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_CHECKS_HPP
#define ICD_ANALYSIS_CHECKS_HPP

#include "analysis.hpp"
#include "enums.hpp"
#include "type_aliases.hpp"

namespace analysis {

namespace {

    template <typename T>
    void assert_eq(T val1, const str_t& name1, T val2, const str_t& name2)
    {
        if (val1 != val2) {
            throw base::exception("Expected " + name1 + " = " + name2
                + "; got " + name1 + " = " + std::to_string(val1)
                + ", " + name2 + " = " + std::to_string(val2));
        }
    }

    template <typename T>
    void assert_gt0(T val, const str_t& name)
    {
        if (!(static_cast<T>(0) < val)) {
            throw base::exception(name + " must be > 0; got " + name
                + " = " + std::to_string(val));
        }
    }

    template <typename T>
    void assert_le(T lesser, const str_t& l_name,
        T greater, const str_t& g_name)
    {
        if (!(lesser <= greater)) {
            throw base::exception("Expected " + l_name + " <= " + g_name
                + "; got " + l_name + " = " + std::to_string(lesser)
                + ", " + g_name + " = " + std::to_string(greater));
        }
    }

    template <typename T>
    void assert_lt(T lesser, const str_t& l_name,
        T greater, const str_t& g_name)
    {
        if (!(lesser < greater)) {
            throw base::exception("Expected " + l_name + " < " + g_name
                + "; got " + l_name + " = " + std::to_string(lesser)
                + ", " + g_name + " = " + std::to_string(greater));
        }
    }

    template <typename T>
    void assert_ptr_not_null(const T* ptr, const str_t& name)
    {
        if (nullptr == ptr) {
            throw base::exception("Pointer to " + name + " is NULL");
        }
    }

    template <typename T>
    void check_array(const T* ptr, size_t size, const str_t& name)
    {
        assert_ptr_not_null(ptr, name);
        if (0 == size) {
            throw base::exception("Pointer to " + name + " is not NULL, "
                + "but size of " + name + " is 0");
        }
    }

    template <typename T>
    void check_array_pair(const T* ptr1, size_t size1, const str_t& name1,
        const T* ptr2, size_t size2, const str_t& name2)
    {
        check_array(ptr1, size1, name1);
        check_array(ptr2, size2, name2);
        assert_eq(size1, "size of " + name1, size2, "size of " + name2);
    }

    void check_fs(real_t fs)
    {
        assert_gt0(fs, "Sample rate");
    }

    void check_fsr(real_t fsr)
    {
        assert_gt0(fsr, "Full-scale range");
    }

    template <typename T>
    void check_resolution(int res)
    {
        const int lib_min = 1;
        const int lib_max = 30;
        const int type_max = 8 * static_cast<int>(sizeof(T));
        assert_le(lib_min, "libary limit", res, "resolution");
        assert_le(res, "resolution", type_max, "data type limit");
        assert_le(res, "resolution", lib_max, "library limit");
    }

    template <typename T>
    bool is_even(T n)
    {
        return 0 == n % 2;
    }

    template <typename T>
    bool is_odd(T n)
    {
        return !is_even(n);
    }

    template <typename T>
    bool is_pow2(T n)
    {
        return (n < 1) ? false : 0 == (n & (n - 1));
    }

} // namespace anonymous

} // namespace analysis

#endif // ICD_ANALYSIS_CHECKS_HPP
