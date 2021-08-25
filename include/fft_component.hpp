/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/fft_component.hpp $
Originator  : pderouni
Revision    : $Revision: 12274 $
Last Commit : $Date: 2020-02-19 06:47:51 -0500 (Wed, 19 Feb 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_FFT_COMPONENT_HPP
#define ICD_ANALYSIS_FFT_COMPONENT_HPP

#include "enums.hpp"
#include "type_aliases.hpp"
#include <memory>
#include <vector>

namespace analysis {

class io_map;

/**
     * @brief The fft_component class
     */
class fft_component {
public:
    using unique_ptr = std::unique_ptr<fft_component>;
    using str_vector = std::vector<str_t>;

public:
    static unique_ptr load(const io_map& state);

public: /// @{ @name Constructors
    fft_component(FFTCompTag tag)
        : m_tag(tag)
    {
    }

    fft_component(const fft_component&) = default;

    /// @}

public: /// @{ @name Destructor
    virtual ~fft_component() = default;

    /// @}

public: /// @{ @name Properties
    FFTCompTag tag() const
    {
        return m_tag;
    }

    FFTCompType type() const
    {
        return type_impl();
    }

    /// @}

public: /// @{ @name Other Member Functions
    unique_ptr clone() const
    {
        return clone_impl();
    }

    std::unique_ptr<io_map> save() const;

    str_vector table_row(int i, const str_t& key) const;

    /// @}

private:
    virtual unique_ptr clone_impl() const = 0;

    virtual str_t param_str() const = 0;

    virtual void save_state(io_map& state) const = 0;

    virtual FFTCompType type_impl() const = 0;

private:
    FFTCompTag m_tag;

}; // class fft_component

/**
     * @brief The fft_band class
     */
class fft_band final : public fft_component {
public:
    static unique_ptr create(FFTCompTag tag,
        const str_t& center,
        const str_t& width,
        bool keep_out,
        bool omit_tones)
    {
        return std::make_unique<fft_band>(
            tag, center, width, keep_out, omit_tones);
    }

public: /// @{ @name Constructors
    fft_band(FFTCompTag tag,
        const str_t& center,
        const str_t& width,
        bool keep_out,
        bool omit_tones)
        : fft_component(tag)
        , m_center(center)
        , m_width(width)
        , m_keep_out(keep_out)
        , m_omit_tones(omit_tones)
    {
    }

    fft_band(const fft_band&) = default;

    /// @}

public: /// @{ @name Destructor
    ~fft_band() = default;

    /// @}

public: /// @{ @name Properties
    str_t center() const
    {
        return m_center;
    }

    bool keep_out() const
    {
        return m_keep_out;
    }

    bool omit_tones() const
    {
        return m_omit_tones;
    }

    str_t width() const
    {
        return m_width;
    }

    /// @}

private: // fft_component overrides
    unique_ptr clone_impl() const override
    {
        return std::make_unique<fft_band>(*this);
    }

    str_t param_str() const override;

    void save_state(io_map& state) const override;

    FFTCompType type_impl() const override
    {
        return FFTCompType::Band;
    }

private:
    str_t m_center;
    str_t m_width;
    bool m_keep_out;
    bool m_omit_tones;

}; // class fft_band

class fft_fixed_tone final : public fft_component {
public:
    static unique_ptr create(FFTCompTag tag,
        const str_t& freq,
        int ssb)
    {
        return std::make_unique<fft_fixed_tone>(tag, freq, ssb);
    }

public: /// @{ @name Constructors
    fft_fixed_tone(FFTCompTag tag, const str_t& freq, int ssb)
        : fft_component(tag)
        , m_freq(freq)
    {
        set_ssb(ssb);
    }

    fft_fixed_tone(const fft_fixed_tone&) = default;

    /// @}

public: /// @{ @name Destructor
    ~fft_fixed_tone() = default;

    /// @}

public: /// @{ @name Properties
    str_t freq() const
    {
        return m_freq;
    }

    int ssb() const
    {
        return m_ssb;
    }

    void set_ssb(int ssb)
    {
        m_ssb = (ssb < 0) ? -1 : ssb;
    }

    /// @}

private: // fft_component overrides
    unique_ptr clone_impl() const override
    {
        return std::make_unique<fft_fixed_tone>(*this);
    }

    str_t param_str() const override;

    void save_state(io_map& state) const override;

    FFTCompType type_impl() const override
    {
        return FFTCompType::FixedTone;
    }

private:
    str_t m_freq;
    int m_ssb;

}; // class fft_fixed_tone

/**
     * @brief The fft_max_tone class
     */
class fft_max_tone final : public fft_component {
public:
    static unique_ptr create(FFTCompTag tag,
        const str_t& center,
        const str_t& width,
        int ssb)
    {
        return std::make_unique<fft_max_tone>(tag, center, width, ssb);
    }

public: /// @{ @name Constructors
    fft_max_tone(FFTCompTag tag,
        const str_t& center,
        const str_t& width,
        int ssb)
        : fft_component(tag)
        , m_center(center)
        , m_width(width)
    {
        set_ssb(ssb);
    }

    fft_max_tone(const fft_max_tone&) = default;

    /// @}

public: /// @{ @name Destructor
    ~fft_max_tone() = default;

    /// @}

public: /// @{ @name Properties
    str_t center() const
    {
        return m_center;
    }

    str_t width() const
    {
        return m_width;
    }

    int ssb() const
    {
        return m_ssb;
    }

    void set_ssb(int ssb)
    {
        m_ssb = (ssb < 0) ? -1 : ssb;
    }

    /// @}

private: // fft_component overrides
    unique_ptr clone_impl() const override
    {
        return std::make_unique<fft_max_tone>(*this);
    }

    str_t param_str() const override;

    void save_state(io_map& state) const override;

    FFTCompType type_impl() const override
    {
        return FFTCompType::MaxTone;
    }

private:
    str_t m_center;
    str_t m_width;
    int m_ssb;

}; // class fft_max_tone

} // namespace analysis

#endif // ICD_ANALYSIS_FFT_COMPONENT_HPP
