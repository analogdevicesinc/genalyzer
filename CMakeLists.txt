cmake_minimum_required(VERSION 3.13.4)
project(
  genalyzer
  VERSION 0.1.2
  LANGUAGES CXX C)

set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL "Build shared libraries")

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

option(BUILD_DOC "Generate documentation" OFF)
option(BUILD_TESTS_EXAMPLES "Build tests and examples" OFF)
option(COVERAGE "Enable coverage tracing when testing" OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  include(CTest)

  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -std=gnu++17 -fext-numeric-literals")
    # If GCC is older than 8.0.0, enable experimental filesystem
    execute_process(
      COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    if(GCC_VERSION VERSION_LESS 8.0.0)
      message(STATUS "GCC version < 8.0.0, enabling experimental filesystem")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs")
      add_definitions(-DEXPERIMENTAL_FILESYSTEM=1)
    endif()
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    if(MINGW)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    elseif(MSVC)
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
    endif()
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++17")
  endif()

  if(BUILD_DOC)
    find_package(Doxygen)
    if(Doxygen_FOUND)
      add_subdirectory(doc)
    else()
      message(STATUS "Doxygen not found, not building docs")
    endif()
  endif()
endif()

if(MSVC)
  add_compile_options(/wd4251 /wd4996)
elseif(MINGW)
  add_compile_options(-Wno-deprecated-declarations)
endif()

option(LIB_CHECK_PYINSTALL "Check for library during python install" OFF)
add_subdirectory(src)
add_subdirectory(bindings)
if(BUILD_TESTS_EXAMPLES)
  add_subdirectory(tests)
  add_subdirectory(examples)
  # Add coverage flags for compiler
  if(COVERAGE)
    if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
      message(FATAL_ERROR "Coverage only supported on Linux")
    endif()
    # Verify that lcov is installed
    find_program(LCOV_PATH lcov)
    if(NOT LCOV_PATH)
      message(FATAL_ERROR "lcov not found! Aborting...")
    endif()
    SET(CMAKE_CXX_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    SET(CMAKE_C_FLAGS "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
    SET(CMAKE_EXE_LINKER_FLAGS "-fprofile-arcs -ftest-coverage")
    # Add custom target to run tests with coverage
    add_custom_target(
      coverage
      COMMAND ${CMAKE_CTEST_COMMAND} --progress &&  lcov -c -d bindings/c/src -o main_coverage.info && genhtml main_coverage.info -o coverage
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

    message(STATUS "Coverage flags enabled")
  endif()
endif()

if(WIN32)
  configure_file(libgenalyzer.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/libgenalyzer.iss @ONLY)
endif()

install(TARGETS genalyzer DESTINATION ${CMAKE_INSTALL_PREFIX})
