#include "real_to_string.hpp"
#include "constants.hpp"

namespace analysis {

namespace {

    str_t real_to_auto_string(real_t n)
    {
        if (!std::isfinite(n)) {
            throw base::exception("Cannot convert non-finite number.");
        }
        std::ostringstream ss;
        ss.unsetf(std::ios_base::floatfield); // equivalent to %g
        ss.precision(real_t_digits10);
        ss << n;
        return ss.str();
    }

} // namespace anonymous

str_t real_to_string(real_t n, NumericFormat format, int precision)
{
    switch (format) {
    case NumericFormat::Eng:
        return real_to_eng_string(n);
    case NumericFormat::Sci:
        return real_to_sci_string(n, precision);
    default: // NumericFormat::Auto
        return real_to_auto_string(n);
    }
}

str_t real_to_eng_string(real_t n)
{
    if (!std::isfinite(n)) {
        throw base::exception("Cannot convert non-finite number.");
    }
    if (0 == n) {
        return "0";
    }
    bool neg = std::signbit(n);
    n = abs(n);
    auto exp3 = floor(log10(n) / 3) * 3; // snap exponent to multiple of 3
    /*
         * The line above may get the wrong exponent if n is very very close
         * to, but less than, 10^exp3.  For example, given
         *     n = 999.999999999999e6
         * the expoonent should be
         *     exp3 = 6
         * but instead exp3 is 9.  This would (wrongly) cause the output string
         * to be 0.999999999999999e9.  6 is the correct value because a double
         * can represent the full precision of this 15 digit example.  One more
         * trailing 9 would (correctly) cause the output string to be
         * 1e9.  The following if statement detects this situation.
         *
         * The cases where this issue arises is due to finite precision in the
         * log10 computation.
         */
    if (pow(10, exp3) > n) {
        // If execution enters this block, the exponent *may* be wrong.
        // Assuming it is, adjust the exponent.
        exp3 -= 3;
        auto n2 = n / pow(10, exp3); // n2 should be 1 <= n2 < 1000
        // Now check the exponent of n2.  The debugger will show n2 < 1000,
        // but if floor(log10(n2)) == 3, then it will be rounded up to
        // 1000 when it is inserted into the ostringstream.  If this is the
        // case, undo the (exp3 -= 3) above.
        if (3 == floor(log10(n2))) {
            exp3 += 3;
        }
    }
    if (-3 > exp3 || 0 < exp3) {
        n *= pow(10, -exp3);
    }
    std::ostringstream ss;
    ss.unsetf(std::ios_base::floatfield); // equivalent to %g
    ss.precision(real_t_digits10);
    if (neg) {
        ss << '-';
    }
    ss << n;
    if (-3 > exp3 || 0 < exp3) {
        ss << 'e' << exp3;
    }
    return ss.str();
}

str_t real_to_sci_string(real_t n, int precision)
{
    if (!std::isfinite(n)) {
        throw base::exception("Cannot convert non-finite number.");
    }
    std::ostringstream ss;
    ss.setf(std::ios_base::scientific);
    const auto max_precision = real_t_digits10 - 1;
    if (0 < precision && max_precision >= precision) {
        ss.precision(precision);
        ss << n;
        return ss.str();
    } else {
        ss.precision(max_precision);
        ss << n;
        auto s = ss.str();
        // trim trailing zeros
        const auto epos = s.rfind('e');
        auto zpos = epos;
        while ('0' == s[zpos - 1]) {
            --zpos;
        }
        if ('.' == s[zpos - 1]) {
            ++zpos;
        }
        s.erase(zpos, epos - zpos);
        return s;
    }
}

} // namespace analysis
