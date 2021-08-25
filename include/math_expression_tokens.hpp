#ifndef ICD_ANALYSIS_MATH_EXPRESSION_TOKENS_HPP
#define ICD_ANALYSIS_MATH_EXPRESSION_TOKENS_HPP

#include "real_to_string.hpp"
#include <regex>
#include <unordered_map>
#include <unordered_set>

namespace analysis {

namespace math_expression_tokens {

    enum class TokenType {
        Number,
        Variable,
        Parenthesis,
        Operator
    };

    enum class ParenType {
        Left,
        Right
    };

    enum class OpType {
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        UPlus,
        UMinus,
        Pow
    };

    enum class OpArity {
        Unary,
        Binary
    };

    enum class OpAssoc {
        Left,
        Right
    };

    enum OpProps {
        OpPropsSymbol = 0,
        OpPropsPrec,
        OpPropsArity,
        OpPropsAssoc
    };

    struct enum_class_hash {

        template <typename T>
        std::size_t operator()(T t) const
        {
            return static_cast<std::size_t>(t);
        }

    }; // struct enum_class_hash

    using op_props = std::tuple<char, int, OpArity, OpAssoc>;

    const std::unordered_map<OpType, op_props, enum_class_hash> operator_lut{
        { OpType::Add, std::make_tuple('+', 0, OpArity::Binary, OpAssoc::Left) },
        { OpType::Sub, std::make_tuple('-', 0, OpArity::Binary, OpAssoc::Left) },
        { OpType::Mul, std::make_tuple('*', 1, OpArity::Binary, OpAssoc::Left) },
        { OpType::Div, std::make_tuple('/', 1, OpArity::Binary, OpAssoc::Left) },
        { OpType::Mod, std::make_tuple('%', 1, OpArity::Binary, OpAssoc::Left) },
        { OpType::UPlus, std::make_tuple('+', 2, OpArity::Unary, OpAssoc::Right) },
        { OpType::UMinus, std::make_tuple('-', 2, OpArity::Unary, OpAssoc::Right) },
        { OpType::Pow, std::make_tuple('^', 2, OpArity::Binary, OpAssoc::Right) }
    };

    inline bool is_operator(char c)
    {
        return ('+' == c) || ('-' == c) || ('*' == c) || ('/' == c) || ('%' == c) || ('^' == c);
    }

    inline bool is_variable(const std::string& var)
    {
        static const std::regex pat{ "[[:alpha:]][[:alnum:]_]*" };
        return std::regex_match(var, pat);
    }

    struct base_token {
        using unique_ptr = std::unique_ptr<base_token>;

        virtual ~base_token() = default;

        virtual unique_ptr clone() const = 0;

        virtual TokenType type() const = 0;

    }; // struct base_token

    struct number_token final : base_token {
        number_token(real_t v)
            : value{ v }
        {
        }

        unique_ptr clone() const
        {
            return std::make_unique<number_token>(*this);
        }

        TokenType type() const
        {
            return TokenType::Number;
        }

        real_t value;

    }; // struct number_token

    struct variable_token final : base_token {
        variable_token(std::string v)
            : variable(std::move(v))
        {
        }

        unique_ptr clone() const
        {
            return std::make_unique<variable_token>(*this);
        }

        TokenType type() const
        {
            return TokenType::Variable;
        }

        std::string variable;

    }; // struct variable_token

    struct paren_token final : base_token {
        paren_token(ParenType p)
            : paren{ p }
            , paren_char((ParenType::Left == paren) ? '(' : ')')
        {
        }

        unique_ptr clone() const
        {
            return std::make_unique<paren_token>(*this);
        }

        TokenType type() const
        {
            return TokenType::Parenthesis;
        }

        ParenType paren;
        char paren_char;

    }; // struct paren_token

    struct operator_token final : base_token {
        operator_token(OpType o)
            : op{ o }
        {
            auto& props = operator_lut.at(op);
            symbol = std::get<OpPropsSymbol>(props);
            prec = std::get<OpPropsPrec>(props);
            arity = std::get<OpPropsArity>(props);
            assoc = std::get<OpPropsAssoc>(props);
        }

        unique_ptr clone() const
        {
            return std::make_unique<operator_token>(*this);
        }

        TokenType type() const
        {
            return TokenType::Operator;
        }

        bool operator<(const operator_token& that) const
        {
            return this->prec < that.prec;
        }

        bool operator<=(const operator_token& that) const
        {
            return this->prec <= that.prec;
        }

        OpType op;
        char symbol;
        int prec;
        OpArity arity;
        OpAssoc assoc;

    }; // struct operator_token

    using infix_tokens_t = std::vector<base_token::unique_ptr>;
    using variable_set = std::unordered_set<std::string>;

    inline void operator<<(infix_tokens_t& tokens, real_t value)
    {
        tokens.push_back(std::make_unique<number_token>(value));
    }

    inline void operator<<(infix_tokens_t& tokens, std::string var)
    {
        tokens.push_back(std::make_unique<variable_token>(std::move(var)));
    }

    inline void operator<<(infix_tokens_t& tokens, ParenType paren)
    {
        tokens.push_back(std::make_unique<paren_token>(paren));
    }

    inline void operator<<(infix_tokens_t& tokens, OpType op)
    {
        tokens.push_back(std::make_unique<operator_token>(op));
    }

    void push_back_operator(infix_tokens_t& tokens, char c,
        OpArity arity = OpArity::Binary);

    infix_tokens_t tokenize_infix(const std::string& infix);

    std::string make_infix_string(
        const infix_tokens_t& tokens,
        NumericFormat format = NumericFormat::Auto);

    variable_set make_variable_set(const infix_tokens_t& tokens);

} // namespace math_expression_tokens

} // namespace analysis

#endif // ICD_ANALYSIS_MATH_EXPRESSION_TOKENS_HPP
