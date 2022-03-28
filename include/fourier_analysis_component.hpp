#ifndef DCANALYSIS_IMPL_FOURIER_ANALYSIS_COMPONENT_HPP
#define DCANALYSIS_IMPL_FOURIER_ANALYSIS_COMPONENT_HPP

#include "enums.hpp"
#include "type_aliases.hpp"

#include <memory>

namespace dcanalysis_impl {

    struct fourier_analysis_component
    {
        using pointer = std::unique_ptr<fourier_analysis_component>;

        fourier_analysis_component(FACompType _type, FACompTag _tag)
            : type (_type),
              tag (_tag)
        {}

        virtual ~fourier_analysis_component() = default;

        pointer clone() const
        {
            return clone_impl();
        }

        bool equals(const fourier_analysis_component& that) const
        {
            return this->equals_impl(that);
        }

        str_t spec() const
        {
            return spec_impl();
        }

        const FACompType type;
        const FACompTag tag;
    
    private:

        virtual pointer clone_impl() const = 0;

        virtual bool equals_impl(const fourier_analysis_component& that) const = 0;

        virtual str_t spec_impl() const = 0;

    }; // class fourier_analysis_component

    struct fa_dc final : public fourier_analysis_component
    {
        static pointer create(int ssb)
        {
            return std::make_unique<fa_dc>(ssb);
        }

        fa_dc(int _ssb)
            : fourier_analysis_component(FACompType::DC, FACompTag::DC),
              ssb (_ssb)
        {}

        ~fa_dc() = default;
    
        const int ssb;

    private:

        pointer clone_impl() const override
        {
            return std::make_unique<fa_dc>(*this);
        }

        bool equals_impl(const fourier_analysis_component& that_obj) const override
        {
            bool equal = (this->type == that_obj.type) && (this->tag == that_obj.tag);
            if (!equal) {
                return false;
            }
            auto& that = static_cast<const fa_dc&>(that_obj);
            return this->ssb == that.ssb;
        }

        str_t spec_impl() const override
        {
            return "SSB= " + std::to_string(ssb);
        }

    }; // class fa_dc

    struct fa_fixed_tone final : public fourier_analysis_component
    {
        static pointer create(FACompTag tag, const str_t& freq, int ssb)
        {
            return std::make_unique<fa_fixed_tone>(tag, freq, ssb);
        }

        fa_fixed_tone(FACompTag tag, const str_t& _freq, int _ssb)
            : fourier_analysis_component(FACompType::FixedTone, tag),
              freq (_freq),
              ssb (_ssb)
        {}

        ~fa_fixed_tone() = default;
    
        const str_t freq;
        const int ssb;

    private:

        pointer clone_impl() const override
        {
            return std::make_unique<fa_fixed_tone>(*this);
        }

        bool equals_impl(const fourier_analysis_component& that_obj) const override
        {
            bool equal = (this->type == that_obj.type) && (this->tag == that_obj.tag);
            if (!equal) {
                return false;
            }
            auto& that = static_cast<const fa_fixed_tone&>(that_obj);
            return (this->freq == that.freq) && (this->ssb == that.ssb);
        }

        str_t spec_impl() const override
        {
            return "F= " + freq + " , SSB= " + std::to_string(ssb);
        }

    }; // class fa_fixed_tone

    struct fa_max_tone final : public fourier_analysis_component
    {
        static pointer create(FACompTag tag, const str_t& center, const str_t& width, int ssb)
        {
            return std::make_unique<fa_max_tone>(tag, center, width, ssb);
        }

        fa_max_tone(FACompTag tag, const str_t& _center, const str_t& _width, int _ssb)
            : fourier_analysis_component(FACompType::MaxTone, tag),
              center (_center),
              width (_width),
              ssb (_ssb)
        {}

        ~fa_max_tone() = default;
    
        const str_t center;
        const str_t width;
        const int ssb;

    private:

        pointer clone_impl() const override
        {
            return std::make_unique<fa_max_tone>(*this);
        }

        bool equals_impl(const fourier_analysis_component& that_obj) const override
        {
            bool equal = (this->type == that_obj.type) && (this->tag == that_obj.tag);
            if (!equal) {
                return false;
            }
            auto& that = static_cast<const fa_max_tone&>(that_obj);
            return (this->center == that.center) &&
                   (this->width  == that.width ) &&
                   (this->ssb    == that.ssb   );
        }

        str_t spec_impl() const override
        {
            return "C= " + center + " , W= " + width + " , SSB= " + std::to_string(ssb);
        }

    }; // class fa_max_tone

    struct fa_wo_tone final : public fourier_analysis_component
    {
        static pointer create(int ssb)
        {
            return std::make_unique<fa_wo_tone>(ssb);
        }

        fa_wo_tone(int _ssb)
            : fourier_analysis_component(FACompType::WOTone, FACompTag::Noise),
              ssb (_ssb)
        {}

        ~fa_wo_tone() = default;
    
        const int ssb;

    private:

        pointer clone_impl() const override
        {
            return std::make_unique<fa_wo_tone>(*this);
        }

        bool equals_impl(const fourier_analysis_component& that_obj) const override
        {
            bool equal = (this->type == that_obj.type) && (this->tag == that_obj.tag);
            if (!equal) {
                return false;
            }
            auto& that = static_cast<const fa_wo_tone&>(that_obj);
            return this->ssb == that.ssb;
        }

        str_t spec_impl() const override
        {
            return "SSB= " + std::to_string(ssb);
        }

    }; // class fa_wo_tone

} // namespace dcanalysis_impl

#endif // DCANALYSIS_IMPL_FOURIER_ANALYSIS_COMPONENT_HPP