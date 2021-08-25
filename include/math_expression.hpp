#ifndef ICD_ANALYSIS_MATH_EXPRESSION_HPP
#define ICD_ANALYSIS_MATH_EXPRESSION_HPP

#include "analysis.hpp"
#include "math_expression_tokens.hpp"

namespace analysis {

class ICD_ANALYSIS_DECL math_expression {
public:
    using variable_set = math_expression_tokens::variable_set;
    using variable_map = std::unordered_map<std::string, real_t>;

public:
    math_expression(const std::string& infix,
        NumericFormat format = NumericFormat::Auto);

    math_expression(const std::string& infix,
        const variable_set& vars,
        NumericFormat format = NumericFormat::Auto);

    math_expression(real_t constant,
        NumericFormat format = NumericFormat::Auto);

    math_expression(const math_expression& expr);

    math_expression(math_expression&& expr);

    ~math_expression() = default;

public:
    math_expression& operator=(const math_expression& expr);

    math_expression& operator=(math_expression&& expr);

public:
    real_t evaluate(const variable_map& lut = variable_map()) const;

    void rename_var(const std::string& old_var,
        const std::string& new_var);

    void set_numeric_format(NumericFormat format)
    {
        if (m_numeric_format != format) {
            m_numeric_format = format;
            m_infix_string = math_expression_tokens::make_infix_string(
                m_infix_tokens, format);
        }
    }

    std::string to_postfix_string() const;

    std::string to_string() const
    {
        return m_infix_string;
    }

    variable_set vars() const
    {
        return m_variables;
    }

public:
    /*
         * Returns true if the expression depends on var, otherwise false.
         */
    bool depends_on(const std::string& var) const
    {
        return (m_variables.end() != m_variables.find(var));
    }

    /*
         * Variable tokens in the expression are checked against the set
         * provided by the user.  Returns true if all variables in the
         * expression are found in the set.  Returns false if the expression
         * has a variable not found in the set.
         */
    bool vars_declared(const variable_set& vars) const;

    /*
         * Same as vars_declared, but takes a variable_map instead.
         */
    bool vars_defined(const variable_map& lut) const;

    /*
         * Variable tokens in the expression are checked against the set
         * provided by the user.  Returns the set of variables in the
         * expression that are not found in the set provided.
         */
    variable_set vars_missing(const variable_set& vars) const;

private:
    using base_token = math_expression_tokens::base_token;
    using infix_tokens_t = math_expression_tokens::infix_tokens_t;
    using postfix_tokens_t = std::vector<const base_token*>;

    static postfix_tokens_t infix_to_postfix(
        const infix_tokens_t& infix_tokens);

private:
    infix_tokens_t m_infix_tokens;
    postfix_tokens_t m_postfix_tokens;
    std::string m_infix_string;
    variable_set m_variables;
    NumericFormat m_numeric_format;

}; // class math_expression

} // namespace analysis

#endif // ICD_ANALYSIS_MATH_EXPRESSION_HPP
