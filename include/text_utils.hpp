/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/text_utils.hpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_TEXT_UTILS_HPP
#define ICD_ANALYSIS_TEXT_UTILS_HPP

#include "analysis.hpp"
#include "enums.hpp"
#include "type_aliases.hpp"
#include <algorithm>
#include <cctype>
#include <string>

#define ICD_DECL ICD_ANALYSIS_DECL

namespace analysis {

namespace utils {

    /// @{ @name Text Utilities

    /**
         * @ingroup Utilities
         * @brief The caseless_char_compare function object
         */
    struct caseless_char_compare {
        using vt = str_t::value_type;
        bool operator()(const vt& c1, const vt& c2) const
        {
            return std::tolower(static_cast<int>(c1)) < std::tolower(static_cast<int>(c2));
        }
    };

    /**
         * @ingroup Utilities
         * @brief The caseless_string_compare function object
         */
    struct caseless_string_compare {
        bool operator()(const str_t& s1, const str_t& s2) const
        {
            return std::lexicographical_compare(s1.begin(), s1.end(),
                s2.begin(), s2.end(),
                caseless_char_compare());
        }
    };

    /// @ingroup Utilities

    ICD_DECL str_t real_to_str(real_t n,
        RealFormat fmt = RealFormat::Auto,
        int prec = -1);

    ICD_DECL str_t real_to_eng(real_t n, int prec = -1);

    ICD_DECL str_t real_to_fix(real_t n, int prec);

    ICD_DECL str_t real_to_sci(real_t n, int prec = -1);

    /**
         * @brief tolower
         * @param[in] s
         * @return
         */
    inline str_t tolower(str_t s)
    {
        using vt = str_t::value_type;
        std::transform(s.begin(), s.end(), s.begin(),
            [](vt c) { return static_cast<vt>(std::tolower(static_cast<int>(c))); });
        return s;
    }

    /**
         * @brief tolower
         * @param[in,out] s
         */
    inline void tolower(str_t& s)
    {
        using vt = str_t::value_type;
        std::transform(s.begin(), s.end(), s.begin(),
            [](vt c) { return static_cast<vt>(std::tolower(static_cast<int>(c))); });
    }

    /**
         * @brief toupper
         * @param[in] s
         * @return
         */
    inline str_t toupper(str_t s)
    {
        using vt = str_t::value_type;
        std::transform(s.begin(), s.end(), s.begin(),
            [](vt c) { return static_cast<vt>(std::toupper(static_cast<int>(c))); });
        return s;
    }

    /**
         * @brief toupper
         * @param[in,out] s
         */
    inline void toupper(str_t& s)
    {
        using vt = str_t::value_type;
        std::transform(s.begin(), s.end(), s.begin(),
            [](vt c) { return static_cast<vt>(std::toupper(static_cast<int>(c))); });
    }

    /// @}

} // namespace utils

} // namespace analysis

#undef ICD_DECL

#endif // ICD_ANALYSIS_TEXT_UTILS_HPP
