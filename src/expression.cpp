#include "expression.hpp"

#include "exceptions.hpp"
#include "utils.hpp"

#include <cctype>
#include <regex>
#include <sstream>
#include <stack>
#include <tuple>

namespace genalyzer_impl { // expression tokens

    enum class TokenType {
        Number, Operator, Parenthesis, Variable
    };

    enum class OpType {
        Add, Sub, Mul, Div, Mod, UPlus, UMinus, Exp
    };

    enum class OpArity {
        Unary, Binary
    };

    enum class OpAssoc {
        Left, Right
    };

    enum OpProps {
        OpPropsSymbol = 0,
        OpPropsPrec,
        OpPropsArity,
        OpPropsAssoc
    };

    enum class ParenType {
        Left, Right
    };

    struct expression_token
    {
        using pointer = std::unique_ptr<expression_token>;

        virtual ~expression_token() = default;

        virtual pointer clone() const = 0;

        virtual TokenType type() const = 0;
    };

    struct num_token final : public expression_token
    {
        num_token(real_t n)
            : num {n}
        {}

        pointer clone() const override
        {
            return std::make_unique<num_token>(*this);
        }

        TokenType type() const override
        {
            return TokenType::Number;
        }

        real_t num;
    };

    struct op_token final : public expression_token
    {
        op_token(OpType o)
            : op {o}
        {
            std::tuple<char, int, OpArity, OpAssoc> props;
            switch (op)
            {
                case OpType::Add    : props = std::make_tuple('+', 0, OpArity::Binary, OpAssoc::Left ); break;
                case OpType::Sub    : props = std::make_tuple('-', 0, OpArity::Binary, OpAssoc::Left ); break;
                case OpType::Mul    : props = std::make_tuple('*', 1, OpArity::Binary, OpAssoc::Left ); break;
                case OpType::Div    : props = std::make_tuple('/', 1, OpArity::Binary, OpAssoc::Left ); break;
                case OpType::Mod    : props = std::make_tuple('%', 1, OpArity::Binary, OpAssoc::Left ); break;
                case OpType::UPlus  : props = std::make_tuple('+', 2, OpArity::Unary,  OpAssoc::Right); break;
                case OpType::UMinus : props = std::make_tuple('-', 2, OpArity::Unary,  OpAssoc::Right); break;
                case OpType::Exp    : props = std::make_tuple('^', 2, OpArity::Binary, OpAssoc::Right); break;
            }
            symbol = std::get<0>(props);
            prec   = std::get<1>(props);
            arity  = std::get<2>(props);
            assoc  = std::get<3>(props);
        }

        pointer clone() const override
        {
            return std::make_unique<op_token>(*this);
        }

        TokenType type() const override
        {
            return TokenType::Operator;
        }

        bool operator<(const op_token& that) const
        {
            return this->prec < that.prec;
        }

        bool operator<=(const op_token& that) const
        {
            return this->prec <= that.prec;
        }

        OpType op;
        char symbol;
        int prec;
        OpArity arity;
        OpAssoc assoc;
    };

    struct paren_token final : public expression_token
    {
        paren_token(ParenType p)
            : paren {p},
              pchar {ParenType::Left == paren ? '(' : ')'}
        {}

        pointer clone() const override
        {
            return std::make_unique<paren_token>(*this);
        }

        TokenType type() const override
        {
            return TokenType::Parenthesis;
        }

        ParenType paren;
        char pchar;
    };

    struct var_token final : public expression_token
    {

        var_token(const str_t& v)
            : var {v}
        {}

        pointer clone() const override
        {
            return std::make_unique<var_token>(*this);
        }

        TokenType type() const override
        {
            return TokenType::Variable;
        }

        str_t var;
    };

} // genalyzer_impl

namespace genalyzer_impl {

    bool is_binary_operator(char c)
    {
        return ('*' == c) || ('/' == c) || ('%' == c) || ('^' == c);
    }

    bool is_operator(char c)
    {
        return ('+' == c) || ('-' == c) || ('*' == c) || ('/' == c) || ('%' == c) || ('^' == c);
    }

    bool is_variable(const str_t& s)
    {
        static const std::regex re ("[[:alpha:]][[:alnum:]_]*");
        return std::regex_match(s, re);
    }

    void operator<<(expression::token_vector& tokens, real_t n)
    {
        tokens.push_back(std::make_unique<num_token>(n));
    }

    void operator<<(expression::token_vector& tokens, OpType o)
    {
        tokens.push_back(std::make_unique<op_token>(o));
    }

    void operator<<(expression::token_vector& tokens, ParenType p)
    {
        tokens.push_back(std::make_unique<paren_token>(p));
    }

    void operator<<(expression::token_vector& tokens, const str_t& v)
    {
        tokens.push_back(std::make_unique<var_token>(v));
    }

    void push_back_operator(expression::token_vector& tokens, char c, OpArity arity)
    {
        OpType op = OpType::Add;
        switch (c)
        {
            case '+' : op = (OpArity::Unary == arity) ? OpType::UPlus : OpType::Add; break;
            case '-' : op = (OpArity::Unary == arity) ? OpType::UMinus : OpType::Sub; break;
            case '*' : op = OpType::Mul; break;
            case '/' : op = OpType::Div; break;
            case '%' : op = OpType::Mod; break;
            case '^' : op = OpType::Exp; break;
        }
        tokens << op;
    }

    expression::var_set get_vars(const expression::token_vector& tokens)
    {
        expression::var_set vars;
        for (const expression::token_ptr& token : tokens) {
            if (TokenType::Variable == token->type()) {
                auto& v = static_cast<const var_token&>(*token);
                vars.insert(v.var);
            }
        }
        return vars;
    }

    void validate_postfix(const expression::token_vector& tokens)
    {
        str_t msg = "validate_postfix : ";
        std::stack<real_t> operands;
        for (const expression::token_ptr& token : tokens) {
            switch (token->type())
            {
                case TokenType::Number :
                case TokenType::Variable :
                    operands.push(0.0);
                    break;
                case TokenType::Operator : {
                    auto& op = static_cast<const op_token&>(*token);
                    if (OpArity::Unary == op.arity) {
                        if (operands.size() < 1) {
                            throw runtime_error(msg + "stack underflow");
                        }
                    } else {
                        if (operands.size() < 2) {
                            throw runtime_error(msg + "stack underflow");
                        } else {
                            operands.pop();
                        }
                    }
                    break;
                }
                case TokenType::Parenthesis :
                    throw runtime_error(msg + "parenthesis found in postfix");
            }
        }
        if (1 < operands.size()) {
            throw runtime_error(msg + "stack overflow");
        }
    }

    expression::token_vector infix_to_postfix(const expression::token_vector& infix_tokens)
    {
        /*
         * Uses Edsger Dijkstra's Shunting Yard algorithm to convert infix to postfix
         * https://en.wikipedia.org/wiki/Shunting-yard_algorithm
         * https://www.chris-j.co.uk/parsing.php
         */
        expression::token_vector postfix_tokens;
        if (infix_tokens.empty()) {
            return postfix_tokens;
        }
        std::stack<expression::token_ptr> op_stack;
        for (const expression::token_ptr& token : infix_tokens) {
            switch (token->type())
            {
                case TokenType::Number :
                case TokenType::Variable :
                    postfix_tokens.push_back(token->clone());
                    break;
                case TokenType::Operator : {
                    auto& op1 = static_cast<const op_token&>(*token);
                    while (!op_stack.empty()) {
                        if (TokenType::Parenthesis == op_stack.top()->type()) {
                            break;
                        }
                        auto& op2 = static_cast<const op_token&>(*op_stack.top());
                        if ((OpAssoc::Left  == op1.assoc && op1 <= op2) ||
                            (OpAssoc::Right == op1.assoc && op1 <  op2)) {
                            postfix_tokens.push_back(op_stack.top()->clone());
                            op_stack.pop();
                        } else {
                            break;
                        }
                    }
                    op_stack.push(token->clone());
                    break;
                } case TokenType::Parenthesis : {
                    auto& p1 = static_cast<const paren_token&>(*token);
                    if (ParenType::Left == p1.paren) {
                        op_stack.push(token->clone());
                    } else {
                        while (!op_stack.empty()) {
                            if (TokenType::Parenthesis == op_stack.top()->type()) {
                                auto& p2 = static_cast<const paren_token&>(*op_stack.top());
                                if (ParenType::Left == p2.paren) {
                                    op_stack.pop();
                                    break;
                                }
                            }
                            postfix_tokens.push_back(op_stack.top()->clone());
                            op_stack.pop();
                        }
                    }
                    break;
                }
            }
        }
        while (!op_stack.empty()) {
            postfix_tokens.push_back(op_stack.top()->clone());
            op_stack.pop();
        }
        validate_postfix(postfix_tokens);
        return postfix_tokens;
    }

    str_t to_string(const expression::token_vector& tokens, FPFormat fmt, int prec)
    {
        std::ostringstream ss;
        for (const expression::token_ptr& token : tokens) {
            switch (token->type())
            {
                case TokenType::Number : {
                    auto& t = static_cast<const num_token&>(*token);
                    ss << genalyzer_impl::to_string(t.num, fmt, prec);
                    break;
                }
                case TokenType::Operator : {
                    auto& t = static_cast<const op_token&>(*token);
                    ss << t.symbol;
                    break;
                }
                case TokenType::Parenthesis : {
                    auto& t = static_cast<const paren_token&>(*token);
                    ss << t.pchar;
                    break;
                }
                case TokenType::Variable : {
                    auto& t = static_cast<const var_token&>(*token);
                    ss << t.var;
                    break;
                }
            }
        }
        return ss.str();
    }

    expression::token_vector tokenize_infix(const str_t& infix_string)
    {
        expression::token_vector tokens;
        char c = 0;
        std::istringstream ss (infix_string);
        int open_parens = 0;
        const char* syntax_error = "tokenize_infix : syntax error";
        while (ss >> c) {
            // Number
            if (std::isdigit(c) || '.' == c) {
                if (!tokens.empty()) {
                    TokenType previous = tokens.back()->type();
                    if (TokenType::Number == previous || TokenType::Variable == previous) {
                        tokens.clear();
                        throw runtime_error(syntax_error);
                    } else if (TokenType::Parenthesis == previous) {
                        auto& p = static_cast<const paren_token&>(*tokens.back());
                        if (ParenType::Right == p.paren) {
                            tokens << OpType::Mul;
                        }
                    }
                }
                ss.unget();
                real_t num = 0.0;
                ss >> num;

#if defined(__APPLE__)
                if (!ss){
                    // Clang doesn't support stringstream to double if contains characters
                    // Need to manually convert
                    std::string tmp = ss.str();
                    ss.clear(); // clear error flags
                    // Remove all non-numeric characters
                    tmp.erase(std::remove_if(tmp.begin(), tmp.end(), [](char c) { return !(std::isdigit(c) || c == '.'); }), tmp.end());
                    if (tmp.empty()) {
                        throw runtime_error("tokenize_infix : invalid numeric value");
                    }
                    num = std::stod(tmp);
                }
#endif
                if (ss) {
                    tokens << num;
                } else {
                    throw runtime_error("tokenize_infix : invalid numeric value");
                }
            }
            // Operator
            else if (is_operator(c)) {
                if (is_binary_operator(c)) {
                    if (tokens.empty()) {
                        throw runtime_error(syntax_error);
                    }
                    TokenType previous = tokens.back()->type();
                    if (TokenType::Operator == previous) {
                        tokens.clear();
                        throw runtime_error(syntax_error);
                    } else if (TokenType::Parenthesis == previous) {
                        auto& p = static_cast<const paren_token&>(*tokens.back());
                        if (ParenType::Left == p.paren) {
                            tokens.clear();
                            throw runtime_error(syntax_error);
                        }
                    }
                    push_back_operator(tokens, c, OpArity::Binary);
                } else {
                    OpArity arity = OpArity::Unary;
                    if (!tokens.empty()) {
                        TokenType previous = tokens.back()->type();
                        if (TokenType::Number == previous || TokenType::Variable == previous) {
                            arity = OpArity::Binary;
                        } else if (TokenType::Parenthesis == previous) {
                            auto& p = static_cast<const paren_token&>(*tokens.back());
                            if (ParenType::Right == p.paren) {
                                arity = OpArity::Binary;
                            }
                        } else if (TokenType::Operator == previous) {
                            auto& o = static_cast<const op_token&>(*tokens.back());
                            if ('+' == o.symbol || '-' == o.symbol) {
                                tokens.clear();
                                throw runtime_error(syntax_error);
                            }
                        }
                    }
                    push_back_operator(tokens, c, arity);
                }
            }
            // Left Parenthesis
            else if ('(' == c) {
                if (!tokens.empty()) {
                    TokenType previous = tokens.back()->type();
                    if (TokenType::Number == previous || TokenType::Variable == previous) {
                        tokens << OpType::Mul;
                    } else if (TokenType::Parenthesis == previous) {
                        auto& p = static_cast<const paren_token&>(*tokens.back());
                        if (ParenType::Right == p.paren) {
                            tokens << OpType::Mul;
                        }
                    }
                }
                tokens << ParenType::Left;
                open_parens += 1;
            }
            // Right Parenthesis
            else if (')' == c) {
                if (tokens.empty()) {
                    throw runtime_error(syntax_error);
                }
                TokenType previous = tokens.back()->type();
                if (TokenType::Operator == previous) {
                    tokens.clear();
                    throw runtime_error(syntax_error);
                } else if (TokenType::Parenthesis == previous) {
                    auto& p = static_cast<const paren_token&>(*tokens.back());
                    if (ParenType::Left == p.paren) {
                        tokens.clear();
                        throw runtime_error(syntax_error);
                    }
                }
                tokens << ParenType::Right;
                open_parens -= 1;
            }
            // Variable
            else if (std::isalpha(c)) {
                if (!tokens.empty()) {
                    TokenType previous = tokens.back()->type();
                    if (TokenType::Number == previous) {
                        tokens << OpType::Mul;
                    } else if (TokenType::Parenthesis == previous) {
                        auto& p = static_cast<const paren_token&>(*tokens.back());
                        if (ParenType::Right == p.paren) {
                            tokens << OpType::Mul;
                        }
                    } else if (TokenType::Variable == previous) {
                        tokens.clear();
                        throw runtime_error(syntax_error);
                    }
                }
                str_t var (1, c);
                while (ss.get(c)) {
                    if (std::isalnum(c) || '_' == c) {
                        var.push_back(c);
                    } else {
                        ss.unget();
                        break;
                    }
                }
                if (is_variable(var)) {
                    tokens << var;
                } else {
                    throw runtime_error("tokenize_infix : invalid variable name");
                }
            }
            // Invalid
            else {
                tokens.clear();
                throw runtime_error("tokenize_infix : invalid token");
            }
        }
        // Check for unmatched parentheses
        if (0 != open_parens) {
            throw runtime_error("tokenize_infix : expression has unmatched parentheses");
        }
        // Last token must not be an operator or left parenthesis
        if (tokens.empty()) {
            throw runtime_error("tokenize_infix : expression has no tokens");
        } else {
            TokenType last = tokens.back()->type();
            if (TokenType::Operator == last) {
                tokens.clear();
                throw runtime_error(syntax_error);
            } else if (TokenType::Parenthesis == last) {
                auto& p = static_cast<const paren_token&>(*tokens.back());
                if (ParenType::Left == p.paren) {
                    tokens.clear();
                    throw runtime_error("tokenize_infix : expression has unmatched parentheses");
                }
            }
        }
        return tokens;
    }

} // namespace genalyzer_impl

namespace genalyzer_impl { // expression class

    expression::expression(const str_t& infix_string)
        : m_infix_tokens (tokenize_infix(infix_string))
    {}

    expression::~expression() = default;

    bool expression::depends_on(const var_set& vars) const
    {
        var_set expr_vars = get_vars(m_infix_tokens);
        for (const str_t& v : vars) {
            if (expr_vars.find(v) != expr_vars.end()) {
                return true;
            }
        }
        return false;
    }

    real_t expression::evaluate(const var_map& vars) const
    {
        str_t msg = "expression::evaluate : ";
        str_t missing_var = vars_defined(vars);
        if (!missing_var.empty()) {
            throw runtime_error(msg + "expression depends on undefined variable, '" + missing_var + "'");
        }
        token_vector postfix_tokens = infix_to_postfix(m_infix_tokens);
        if (postfix_tokens.empty()) {
            throw runtime_error(msg + "empty expression");
        }
        std::stack<real_t> operands;
        for (const token_ptr& token : postfix_tokens) {
            switch (token->type())
            {
                case TokenType::Number : {
                    auto& t = static_cast<const num_token&>(*token);
                    operands.push(t.num);
                    break;
                }
                case TokenType::Variable : {
                    auto& t = static_cast<const var_token&>(*token);
                    operands.push(vars.at(t.var));
                    break;
                }
                case TokenType::Operator : {
                    auto& t = static_cast<const op_token&>(*token);
                    real_t result = 0.0;
                    if (OpArity::Unary == t.arity) {
                        real_t value = operands.top();
                        operands.pop();
                        switch (t.op)
                        {
                            case OpType::UPlus :
                                result =  value;
                                break;
                            case OpType::UMinus :
                                result = -value;
                                break;
                            default:
                                break;
                        }
                    } else {
                        real_t rval = operands.top();
                        operands.pop();
                        real_t lval = operands.top();
                        operands.pop();
                        switch (t.op)
                        {
                            case OpType::Add :
                                result = lval + rval;
                                break;
                            case OpType::Sub :
                                result = lval - rval;
                                break;
                            case OpType::Mul :
                                result = lval * rval;
                                break;
                            case OpType::Div :
                                if (0.0 == rval) {
                                    throw runtime_error(msg + "divide by 0");
                                }
                                result = lval / rval;
                                break;
                            case OpType::Mod :
                                if (0.0 == rval) {
                                    throw runtime_error(msg + "divide by 0");
                                }
                                result = std::fmod(lval, rval);
                                break;
                            case OpType::Exp :
                                result = std::pow(lval, rval);
                                break;
                            default:
                                break;
                        }
                    }
                    operands.push(result);
                    break;
                }
                default:
                    break;
            }
        }
        if (1 != operands.size()) {
            throw runtime_error(msg + "stack error");
        }
        return operands.top();
    }

    str_t expression::to_postfix_string(FPFormat fmt, int prec) const
    {
        token_vector postfix_tokens = infix_to_postfix(m_infix_tokens);
        return genalyzer_impl::to_string(postfix_tokens, fmt, prec);
    }
    
    str_t expression::to_string(FPFormat fmt, int prec) const
    {
        return genalyzer_impl::to_string(m_infix_tokens, fmt, prec);
    }

    expression::var_set expression::vars() const
    {
        return get_vars(m_infix_tokens);
    }

    str_t expression::vars_defined(const var_map& vars) const
    {
        var_set expr_vars = get_vars(m_infix_tokens);
        for (const str_t& v : expr_vars) {
            if (vars.end() == vars.find(v)) {
                return v;
            }
        }
        return "";
    }
    
} // namespace genalyzer_impl