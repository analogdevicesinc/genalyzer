#include "math_expression.hpp"
#include <stack>

namespace analysis {

using namespace math_expression_tokens;

math_expression::math_expression(const std::string& infix,
    NumericFormat format)
    : m_infix_tokens(tokenize_infix(infix))
    , m_postfix_tokens(infix_to_postfix(m_infix_tokens))
    , m_infix_string(make_infix_string(m_infix_tokens, format))
    , m_variables(make_variable_set(m_infix_tokens))
    , m_numeric_format{ format }
{
}

math_expression::math_expression(const std::string& infix,
    const variable_set& vars,
    NumericFormat format)
    : math_expression(infix, format)
{
    if (!vars_declared(vars)) {
        throw base::exception("Expression has undeclared variables.");
    }
}

math_expression::math_expression(real_t constant, NumericFormat format)
    : math_expression(real_to_string(constant, format), format)
{
}

math_expression::math_expression(const math_expression& expr)
    : m_infix_tokens{}
    , m_postfix_tokens{}
    , m_infix_string(expr.m_infix_string)
    , m_variables(expr.m_variables)
    , m_numeric_format{ expr.m_numeric_format }
{
    for (const auto& token : expr.m_infix_tokens) {
        m_infix_tokens.push_back(token->clone());
    }
    m_postfix_tokens = infix_to_postfix(m_infix_tokens);
}

math_expression::math_expression(math_expression&& expr)
    : m_infix_tokens(std::move(expr.m_infix_tokens))
    , m_postfix_tokens(std::move(expr.m_postfix_tokens))
    , m_infix_string(std::move(expr.m_infix_string))
    , m_variables(std::move(expr.m_variables))
    , m_numeric_format{ expr.m_numeric_format }
{
    expr.m_infix_tokens.clear();
    expr.m_postfix_tokens.clear();
    expr.m_infix_string.clear();
    expr.m_variables.clear();
}

math_expression& math_expression::operator=(const math_expression& expr)
{
    if (&expr != this) {
        math_expression the_copy{ expr };
        std::swap(*this, the_copy);
    }
    return *this;
}

math_expression& math_expression::operator=(math_expression&& expr)
{
    if (&expr != this) {
        m_infix_tokens.swap(expr.m_infix_tokens);
        m_postfix_tokens.swap(expr.m_postfix_tokens);
        m_infix_string.swap(expr.m_infix_string);
        m_variables.swap(expr.m_variables);
        std::swap(m_numeric_format, expr.m_numeric_format);
    }
    return *this;
}

real_t math_expression::evaluate(const variable_map& lut) const
{
    if (!vars_defined(lut)) {
        throw base::exception("Expression has undefined variables.");
    }
    std::stack<real_t> operands;
    for (const auto& token : m_postfix_tokens) { // token is a raw pointer
        switch (token->type()) {
        case TokenType::Number: {
            auto& t = static_cast<const number_token&>(*token);
            operands.push(t.value);
            break;
        }
        case TokenType::Variable: {
            auto& t = static_cast<const variable_token&>(*token);
            operands.push(lut.at(t.variable));
            break;
        }
        case TokenType::Operator: {
            auto& t = static_cast<const operator_token&>(*token);
            real_t result = 0.0;
            if (t.arity == OpArity::Unary) {
                real_t value = operands.top();
                operands.pop();
                switch (t.op) {
                case OpType::UPlus:
                    result = value;
                    break;
                case OpType::UMinus:
                    result = -value;
                    break;
                }
            } else if (t.arity == OpArity::Binary) {
                real_t rval = operands.top();
                operands.pop();
                real_t lval = operands.top();
                operands.pop();
                switch (t.op) {
                case OpType::Add:
                    result = lval + rval;
                    break;
                case OpType::Sub:
                    result = lval - rval;
                    break;
                case OpType::Mul:
                    result = lval * rval;
                    break;
                case OpType::Div:
                    if (0.0 == rval) {
                        throw base::exception("Divide by 0.");
                    }
                    result = lval / rval;
                    break;
                case OpType::Mod:
                    if (0.0 == rval) {
                        throw base::exception("Divide by 0.");
                    }
                    result = fmod(lval, rval);
                    break;
                case OpType::Pow:
                    result = pow(lval, rval);
                    break;
                }
            }
            operands.push(result);
            break;
        }
        }
    }
    if (operands.size() != 1) {
        throw base::exception("Stack error.");
    }
    return operands.top();
}

void math_expression::rename_var(const std::string& old_var,
    const std::string& new_var)
{
    if (!depends_on(old_var)) {
        return;
    }
    if (!is_variable(new_var)) {
        throw base::exception("Invalid variable.");
    }
    m_variables.clear();
    for (auto& token : m_infix_tokens) { // token is a unique_ptr
        if (token->type() == TokenType::Variable) {
            auto& t = static_cast<variable_token&>(*token);
            if (t.variable == old_var) {
                t.variable = new_var;
            }
            m_variables.emplace(t.variable);
        }
    }
    m_infix_string = make_infix_string(m_infix_tokens, m_numeric_format);
}

std::string math_expression::to_postfix_string() const
{
    std::ostringstream ss;
    for (const auto& token : m_postfix_tokens) { // token is a raw pointer
        switch (token->type()) {
        case TokenType::Number: {
            auto& t = static_cast<const number_token&>(*token);
            ss << real_to_string(t.value, m_numeric_format);
            break;
        }
        case TokenType::Variable: {
            auto& t = static_cast<const variable_token&>(*token);
            ss << t.variable;
            break;
        }
        case TokenType::Parenthesis: {
            auto& t = static_cast<const paren_token&>(*token);
            ss << t.paren_char;
            break;
        }
        case TokenType::Operator: {
            auto& t = static_cast<const operator_token&>(*token);
            ss << t.symbol;
            break;
        }
        }
        ss << ' ';
    }
    return ss.str();
}

bool math_expression::vars_declared(const variable_set& vars) const
{
    if (m_variables.size() > vars.size()) {
        return false;
    }
    for (const auto& var : m_variables) {
        if (vars.end() == vars.find(var)) {
            return false;
        }
    }
    return true;
}

bool math_expression::vars_defined(const variable_map& lut) const
{
    if (m_variables.size() > lut.size()) {
        return false;
    }
    for (const auto& var : m_variables) {
        if (lut.end() == lut.find(var)) {
            return false;
        }
    }
    return true;
}

variable_set math_expression::vars_missing(const variable_set& vars) const
{
    variable_set missing;
    for (const auto& var : m_variables) {
        if (vars.end() == vars.find(var)) {
            missing.emplace(var);
        }
    }
    return missing;
}

/*
     * Uses Edsger Dijkstra's "Shunting Yard" Algorithm to convert an infix
     * expression to a postfix (RPN) expression.
     * https://en.wikipedia.org/wiki/Shunting-yard_algorithm
     * http://www.chris-j.co.uk/parsing.php
     */
math_expression::postfix_tokens_t math_expression::infix_to_postfix(
    const infix_tokens_t& infix_tokens)
{
    if (infix_tokens.empty()) {
        throw base::exception("Got empty set of tokens.");
    }
    postfix_tokens_t postfix_tokens;
    std::stack<const base_token*> op_stack;
    for (auto& token : infix_tokens) { // token is a unique_ptr
        switch (token->type()) {
        case TokenType::Number:
        case TokenType::Variable:
            postfix_tokens.push_back(token.get());
            break;
        case TokenType::Parenthesis: {
            auto& p = static_cast<const paren_token&>(*token);
            if (p.paren == ParenType::Left) {
                op_stack.push(token.get());
            } else { // right parenthesis
                while (!op_stack.empty()) {
                    if (op_stack.top()->type() == TokenType::Parenthesis) {
                        auto& p2 = static_cast<const paren_token&>(*op_stack.top());
                        if (p2.paren == ParenType::Left) {
                            op_stack.pop();
                            break;
                        }
                    }
                    postfix_tokens.push_back(op_stack.top());
                    op_stack.pop();
                }
            }
            break;
        }
        case TokenType::Operator: {
            auto& op = static_cast<const operator_token&>(*token);
            while (!op_stack.empty()) {
                if (op_stack.top()->type() == TokenType::Parenthesis) {
                    break; // note: could only be left parenthesis
                }
                auto& op2 = static_cast<const operator_token&>(*op_stack.top());
                if ((op.assoc == OpAssoc::Left && op <= op2) || (op.assoc == OpAssoc::Right && op < op2)) {
                    postfix_tokens.push_back(op_stack.top());
                    op_stack.pop();
                } else {
                    break;
                }
            }
            op_stack.push(token.get());
            break;
        }
        }
    }
    while (!op_stack.empty()) {
        postfix_tokens.push_back(op_stack.top());
        op_stack.pop();
    }
    // validate; unsure if necessary, but easy to check
    std::stack<real_t> operands;
    for (const auto& token : postfix_tokens) { // token is a raw pointer
        switch (token->type()) {
        case TokenType::Number:
        case TokenType::Variable:
            operands.push(0.0);
            break;
        case TokenType::Operator: {
            auto& op = static_cast<const operator_token&>(*token);
            if (op.arity == OpArity::Unary) {
                if (operands.size() < 1) {
                    throw base::exception("Stack underflow.");
                }
            } else if (op.arity == OpArity::Binary) {
                if (operands.size() < 2) {
                    throw base::exception("Stack underflow.");
                } else {
                    operands.pop();
                }
            }
            break;
        }
        default:
            throw base::exception("Stack has invalid token.");
        }
    }
    if (operands.size() > 1) {
        throw base::exception("Stack overflow.");
    }
    return postfix_tokens;
}

} // namespace analysis
