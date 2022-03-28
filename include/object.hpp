#ifndef DCANALYSIS_IMPL_OBJECT_HPP
#define DCANALYSIS_IMPL_OBJECT_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

#include <memory>

namespace dcanalysis_impl {

    class object
    {
    public:

        using pointer = std::shared_ptr<object>;

    public:

        virtual ~object() = default;

    public:

        bool equals(const object& that) const
        {
            return this->equals_impl(that);
        }

        ObjectType object_type() const
        {
            return object_type_impl();
        }

        void save(const str_t& filename) const
        {
            save_impl(filename);
        }

        str_t to_string() const
        {
            return to_string_impl();
        }

    private:

        virtual bool equals_impl(const object& that) const = 0;

        virtual ObjectType object_type_impl() const = 0;

        virtual void save_impl(const str_t& filename) const = 0;

        virtual str_t to_string_impl() const = 0;

    }; // class object

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_OBJECT_HPP