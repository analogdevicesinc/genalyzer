#include "math_expression_tokens.hpp"

namespace analysis {

namespace math_expression_tokens {

    void push_back_operator(infix_tokens_t& tokens, char c, OpArity arity)
    {
        OpType op;
        switch (c) {
        case '+':
            if (OpArity::Unary == arity) {
                op = OpType::UPlus;
            } else {
                op = OpType::Add;
            }
            break;
        case '-':
            if (OpArity::Unary == arity) {
                op = OpType::UMinus;
            } else {
                op = OpType::Sub;
            }
            break;
        case '*':
            op = OpType::Mul;
            break;
        case '/':
            op = OpType::Div;
            break;
        case '%':
            op = OpType::Mod;
            break;
        case '^':
            op = OpType::Pow;
            break;
        default:
            throw base::exception("Unknown operator.");
        }
        tokens << op;
    }

    infix_tokens_t tokenize_infix(const std::string& infix)
    {
        if (infix.empty()) {
            throw base::exception("Got empty string.");
        }
        infix_tokens_t tokens;
        int open_parens = 0;
        std::istringstream ss(infix);
        for (char c; ss >> c;) {
            // Number
            if (isdigit(c) || '.' == c) {
                if (!tokens.empty()) {
                    auto prev_type = tokens.back()->type();
                    if (prev_type == TokenType::Number || prev_type == TokenType::Variable) {
                        tokens.clear();
                        throw base::exception("Expression syntax error.");
                    } else if (prev_type == TokenType::Parenthesis) {
                        auto& p = static_cast<const paren_token&>(*tokens.back());
                        if (p.paren == ParenType::Right) {
                            tokens << OpType::Mul;
                        }
                    }
                }
                ss.unget();
                real_t value;
                ss >> value;
                if (ss) {
                    tokens << value;
                } else {
                    throw base::exception("Invalid numeric value.");
                }
            }
            // Variable
            else if (isalpha(c)) {
                if (!tokens.empty()) {
                    auto prev_type = tokens.back()->type();
                    if (prev_type == TokenType::Number) {
                        tokens << OpType::Mul;
                    } else if (prev_type == TokenType::Variable) {
                        tokens.clear();
                        throw base::exception("Expression syntax error.");
                    } else if (prev_type == TokenType::Parenthesis) {
                        auto& p = static_cast<const paren_token&>(*tokens.back());
                        if (p.paren == ParenType::Right) {
                            tokens << OpType::Mul;
                        }
                    }
                }
                std::string var(1, c);
                while (ss.get(c)) {
                    if (isalnum(c) || '_' == c) {
                        var.push_back(c);
                    } else {
                        ss.unget();
                        break;
                    }
                }
                if (is_variable(var)) {
                    tokens << std::move(var);
                } else {
                    throw base::exception("Invalid variable.");
                }
            }
            // Left Parenthesis
            else if ('(' == c) {
                if (!tokens.empty()) {
                    auto prev_type = tokens.back()->type();
                    if (prev_type == TokenType::Number || prev_type == TokenType::Variable) {
                        tokens << OpType::Mul;
                    } else if (prev_type == TokenType::Parenthesis) {
                        auto& p = static_cast<const paren_token&>(*tokens.back());
                        if (p.paren == ParenType::Right) {
                            tokens << OpType::Mul;
                        }
                    }
                }
                tokens << ParenType::Left;
                ++open_parens;
            }
            // Right Parenthesis
            else if (')' == c) {
                if (tokens.empty()) {
                    throw base::exception("Expression syntax error.");
                }
                auto prev_type = tokens.back()->type();
                if (prev_type == TokenType::Operator) {
                    tokens.clear();
                    throw base::exception("Expression syntax error.");
                } else if (prev_type == TokenType::Parenthesis) {
                    auto& p = static_cast<const paren_token&>(*tokens.back());
                    if (p.paren == ParenType::Left) {
                        tokens.clear();
                        throw base::exception("Expression syntax error.");
                    }
                }
                tokens << ParenType::Right;
                --open_parens;
            }
            // Operator
            else if (is_operator(c)) {
                // binary-only operators
                if ('*' == c || '/' == c || '%' == c || '^' == c) {
                    if (tokens.empty()) {
                        throw base::exception("Expression syntax error.");
                    }
                    auto prev_type = tokens.back()->type();
                    if (prev_type == TokenType::Operator) {
                        tokens.clear();
                        throw base::exception("Expression syntax error.");
                    } else if (prev_type == TokenType::Parenthesis) {
                        auto& p = static_cast<const paren_token&>(*tokens.back());
                        if (p.paren == ParenType::Left) {
                            tokens.clear();
                            throw base::exception("Expression syntax error.");
                        }
                    }
                    push_back_operator(tokens, c);
                }
                // operators that can be unary or binary
                else if ('+' == c || '-' == c) {
                    auto arity = OpArity::Unary;
                    if (!tokens.empty()) {
                        auto prev_type = tokens.back()->type();
                        if (prev_type == TokenType::Number || prev_type == TokenType::Variable) {
                            arity = OpArity::Binary;
                        } else if (prev_type == TokenType::Parenthesis) {
                            auto& p = static_cast<const paren_token&>(*tokens.back());
                            if (p.paren == ParenType::Right) {
                                arity = OpArity::Binary;
                            }
                        } else if (prev_type == TokenType::Operator) {
                            auto& op = static_cast<const operator_token&>(*tokens.back());
                            if (op.symbol == '+' || op.symbol == '-') {
                                tokens.clear();
                                throw base::exception("Expression syntax error.");
                            }
                        }
                    }
                    push_back_operator(tokens, c, arity);
                }
            }
            // Invalid Token
            else {
                tokens.clear();
                throw base::exception("Invalid token.");
            }
        }
        // check all parentheses matched
        if (0 != open_parens) {
            tokens.clear();
            throw base::exception("Expression parentheses mismatch.");
        }
        // last token cannot be '(' or Operator
        auto last_type = tokens.back()->type();
        if (TokenType::Parenthesis == last_type) {
            auto& p = static_cast<const paren_token&>(*tokens.back());
            if (p.paren == ParenType::Left) {
                tokens.clear();
                throw base::exception("Expression parentheses mismatch.");
            }
        } else if (TokenType::Operator == last_type) {
            tokens.clear();
            throw base::exception("Expression syntax error.");
        }
        return tokens;
    }

    std::string make_infix_string(const infix_tokens_t& tokens,
        NumericFormat format)
    {
        std::ostringstream ss;
        for (const auto& token : tokens) { // token is a unique_ptr
            switch (token->type()) {
            case TokenType::Number: {
                auto& t = static_cast<const number_token&>(*token);
                ss << real_to_string(t.value, format);
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
        }
        return ss.str();
    }

    variable_set make_variable_set(const infix_tokens_t& tokens)
    {
        variable_set vars;
        for (const auto& token : tokens) { // token is a unique_ptr
            if (token->type() == TokenType::Variable) {
                auto& t = static_cast<const variable_token&>(*token);
                vars.emplace(t.variable);
            }
        }
        return vars;
    }

} // namespace math_expression_tokens

} // namespace analysis
