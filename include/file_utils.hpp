/*
File        : $HeadURL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/src/analysis/file_utils.hpp $
Originator  : pderouni
Revision    : $Revision: 12191 $
Last Commit : $Date: 2020-01-17 15:04:50 -0500 (Fri, 17 Jan 2020) $
Last Editor : $Author: pderouni $
*/

#ifndef ICD_ANALYSIS_FILE_UTILS_HPP
#define ICD_ANALYSIS_FILE_UTILS_HPP

#include "analysis.hpp"
#include "enums.hpp"
#include <map>

namespace analysis {

namespace utils {

    /// @{ @name File Utilities

    /// @ingroup Utilities

    /// File extension map
    const std::map<FileType, std::string> file_ext_map{
        { FileType::Bin, "bin" },
        { FileType::Json, "json" },
        { FileType::Xml, "xml" },
        { FileType::Yaff, "yaff" }
    };

    /**
         * @brief has_file_ext
         * @param[in] filename
         * @param[in] filetype
         * @return
         */
    inline bool has_file_ext(const std::string& filename,
        FileType filetype)
    {
        std::string suffix = '.' + file_ext_map.at(filetype);
        if (filename.size() > suffix.size()) {
            auto eq = filename.compare(filename.size() - suffix.size(),
                suffix.size(), suffix);
            return 0 == eq;
        }
        return false;
    }

    /**
         * @brief has_file_ext
         * @param[in] filename
         * @return
         */
    inline bool has_file_ext(const std::string& filename)
    {
        for (const auto& kv : file_ext_map) {
            if (has_file_ext(filename, kv.first)) {
                return true;
            }
        }
        return false;
    }

    /**
         * @brief append_file_ext
         * @param[in] filename
         * @param[in] filetype
         * @return
         */
    inline std::string append_file_ext(const std::string& filename,
        FileType filetype)
    {
        return has_file_ext(filename, filetype)
            ? filename
            : filename + '.' + file_ext_map.at(filetype);
    }

    /**
         * @brief append_file_ext
         * @param[in,out] filename
         * @param[in] filetype
         */
    inline void append_file_ext(std::string& filename, FileType filetype)
    {
        if (!has_file_ext(filename, filetype)) {
            filename += '.' + file_ext_map.at(filetype);
        }
    }

    /**
         * @brief get_file_type
         * @param[in] filename
         * @return
         */
    inline FileType get_file_type(const std::string& filename)
    {
        for (const auto& kv : file_ext_map) {
            if (has_file_ext(filename, kv.first)) {
                return kv.first;
            }
        }
        throw base::exception("Unknown file type.");
    }

    /// @}

} // namespace utils

} // namespace analysis

#endif // ICD_ANALYSIS_FILE_UTILS_HPP
