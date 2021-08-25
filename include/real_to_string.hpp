#ifndef ICD_ANALYSIS_REAL_TO_STRING_HPP
#define ICD_ANALYSIS_REAL_TO_STRING_HPP

#include "analysis.hpp"
#include "type_aliases.hpp"

#define ICD_DECL ICD_ANALYSIS_DECL

namespace analysis {

enum class NumericFormat {
    Auto,
    Eng,
    Sci
};

ICD_DECL str_t real_to_string(real_t n,
    NumericFormat format = NumericFormat::Auto,
    int precision = -1);

ICD_DECL str_t real_to_eng_string(real_t n);

ICD_DECL str_t real_to_sci_string(real_t n, int precision = -1);

} // namespace analysis

#undef ICD_DECL

#endif // ICD_ANALYSIS_REAL_TO_STRING_HPP
