/**
 * @file $URL: https://swdev.cld.analog.com/svn/projects/icdev/sandbox/users/pderouni/platform-upgrades/includes/pragmas.hpp $
 * @version $Revision: 12448 $
 * @date $Date: 2020-04-20 22:29:33 -0400 (Mon, 20 Apr 2020) $
 * @author $Author: kshresth $
 * @brief
 */

#ifndef ICD_INCLUDE_PRAGMAS_HPP
#define ICD_INCLUDE_PRAGMAS_HPP

#if defined(__GNUC__)
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

/*
 * Drop compiler to level 3 and save current level
 *   4100 unused variables
 *   4172 returning address of local variable or temporary
 *   4244 conversion, possible loss of data
 *   4267 possible loss of data
 *   4275 non dll-interface class while using boost::python::numpy
 *   4800 'int' : forcing value to bool 'true' or 'false'
 */
#if defined(WIN32) || defined(WIN64)
#define ICD_BOOST_DISABLE_WARNINGS_BEGIN                  \
    __pragma(warning(push, 3))                            \
        __pragma(warning(disable : 4100))                 \
            __pragma(warning(disable : 4172))             \
                __pragma(warning(disable : 4244))         \
                    __pragma(warning(disable : 4267))     \
                        __pragma(warning(disable : 4275)) \
                            __pragma(warning(disable : 4800))
#define ICD_BOOST_DISABLE_WARNINGS_END \
    __pragma(warning(pop))
#else
#define ICD_BOOST_DISABLE_WARNINGS_BEGIN                                \
    _Pragma("GCC diagnostic push")                                      \
        _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"") \
            _Pragma("GCC diagnostic ignored \"-Wunused-local-typedefs\"")
#define ICD_BOOST_DISABLE_WARNINGS_END \
    _Pragma("GCC diagnostic pop")
#endif

/*
 * Drop compiler to level 3 and save current level
 * 4365 conversion from 'type_1' to 'type_2', signed/unsigned mismatch
 * 4714 function marked as __forceinline not inlined
 */
#if defined(WIN32) || defined(WIN64)
#define ICD_EIGEN_DISABLE_WARNINGS_BEGIN  \
    __pragma(warning(push, 3))            \
        __pragma(warning(disable : 4365)) \
            __pragma(warning(disable : 4714))
#define ICD_EIGEN_DISABLE_WARNINGS_END \
    __pragma(warning(pop))
#else
#if (GCC_VERSION >= 70200)
#define ICD_EIGEN_DISABLE_WARNINGS_BEGIN \
    _Pragma("GCC diagnostic push")       \
        _Pragma("GCC diagnostic ignored \"-Wint-in-bool-context\"")
#else
#define ICD_EIGEN_DISABLE_WARNINGS_BEGIN \
    _Pragma("GCC diagnostic push")
#endif
#define ICD_EIGEN_DISABLE_WARNINGS_END \
    _Pragma("GCC diagnostic pop")
#endif

/*
 * Drop compiler to level 3 and save current level
 *   4005 macro redefinition warning
 *   4127 conditional expression is constant
 *   4244 conversion, possible loss of data
 *   4275 non dll-interface class
 *   4512 assignment operator could not be generated
 *   4718 recursive call has no side effects, deleting
 *   4800 forcing value to bool 'true' or 'false' (performance warning)
 */
#if defined(WIN32) || defined(WIN64)
#define ICD_QT_DISABLE_WARNINGS_BEGIN                         \
    __pragma(warning(push, 3))                                \
        __pragma(warning(disable : 4005))                     \
            __pragma(warning(disable : 4127))                 \
                __pragma(warning(disable : 4244))             \
                    __pragma(warning(disable : 4275))         \
                        __pragma(warning(disable : 4512))     \
                            __pragma(warning(disable : 4718)) \
                                __pragma(warning(disable : 4800))
#define ICD_QT_DISABLE_WARNINGS_END \
    __pragma(warning(pop))
#else
#define ICD_QT_DISABLE_WARNINGS_BEGIN
#define ICD_QT_DISABLE_WARNINGS_END
#endif

#endif // ICD_INCLUDE_PRAGMAS_HPP
