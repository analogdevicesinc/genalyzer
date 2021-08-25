/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/text_utils.cpp $
Originator  : pderouni
Revision    : $Revision: 12767 $
Last Commit : $Date: 2020-08-24 16:38:41 -0400 (Mon, 24 Aug 2020) $
Last Editor : $Author: pderouni $
*/

#include "text_utils.hpp"
#include "constants.hpp"
#include <iomanip>

namespace analysis {

namespace utils {

    namespace {

        str_t real_to_auto(real_t n)
        {
            if (!std::isfinite(n)) {
                throw base::exception("Cannot convert non-finite number");
            }
            std::ostringstream oss;
            oss << std::defaultfloat
                << std::setprecision(real_t_digits10)
                << n;
            return oss.str();
        }

    } // namespace anonymous

    str_t real_to_str(real_t n, RealFormat fmt, int prec)
    {
        switch (fmt) {
        case RealFormat::Eng:
            return real_to_eng(n, prec);
        case RealFormat::Fix:
            return real_to_fix(n, prec);
        case RealFormat::Sci:
            return real_to_sci(n, prec);
        default:
            return real_to_auto(n);
        }
    }

    str_t real_to_eng(real_t n, int prec)
    {
        static_cast<void>(n);
        static_cast<void>(prec);
        return "tbi";
    }

    str_t real_to_fix(real_t n, int prec)
    {
        if (!std::isfinite(n)) {
            throw base::exception("Cannot convert non-finite number");
        }
        std::ostringstream oss;
        if (prec < 0) {
            prec = 0;
        }
        oss << std::fixed << std::setprecision(prec) << n;
        return oss.str();
    }

    str_t real_to_sci(real_t n, int prec)
    {
        static_cast<void>(n);
        static_cast<void>(prec);
        return "tbi";
    }

} // namespace utils

} // namespace analysis
