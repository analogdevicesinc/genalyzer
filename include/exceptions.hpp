#ifndef DCANALYSIS_IMPL_EXCEPTIONS_HPP
#define DCANALYSIS_IMPL_EXCEPTIONS_HPP

#include <stdexcept>

namespace dcanalysis_impl {

    class logic_error final : public std::logic_error
    {
        using std::logic_error::logic_error;
    };

    class runtime_error final : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_EXCEPTIONS_HPP