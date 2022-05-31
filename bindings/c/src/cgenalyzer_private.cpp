#include "cgenalyzer_private.h"

namespace util {

    size_t terminated_size(size_t string_size)
    {
        return string_size + (util::gn_null_terminate ? 1 : 0);
    }

    void fill_string_buffer(
        const char* src,        // Pointer to source
        size_t src_size,        // Size of source; should not count null-terminator, if it exists
        char* dst,              // Pointer to destination
        size_t dst_size         // Size of destination
        )
    {
        if (nullptr == src) {
            throw std::runtime_error("fill_string_buffer : source is NULL");
        }
        if (nullptr == dst) {
            throw std::runtime_error("fill_string_buffer : destination is NULL");
        }
        if (dst_size < terminated_size(src_size)) {
            throw std::runtime_error("fill_string_buffer : destination too small");
        }
        for (size_t i = 0; i < src_size; ++i) {
            dst[i] = src[i];
        }
        if (gn_null_terminate) {
            dst[src_size] = '\0';
        }
    }
    
    std::string get_object_key_from_filename(const std::string& filename)
    {
        static const std::regex re {"(" + gn::manager::key_pattern + ")[.]json$", std::regex::icase};
        std::smatch matches;
        if (std::regex_search(filename, matches, re)) {
            if (1 == matches.size()) {
                throw std::runtime_error("unable to derive object key from filename '" + filename + "'");
            }
            return matches[1].str();
        } else {
            throw std::runtime_error("invalid filename '" + filename + "'");
        }
    }

} // namespace util