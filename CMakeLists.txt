cmake_minimum_required(VERSION 3.13.4)
project(
  genalyzer
  VERSION 0.0.1
  LANGUAGES CXX C)

set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL "Build shared libraries")

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

option(BUILD_DOC "Generate documentation" OFF)
option(BUILD_TESTS_EXAMPLES "Build tests and examples" OFF)
option(PYTHON_BINDINGS "Generate python bindings" OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  include(CTest)

  if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -std=gnu++17 -fext-numeric-literals")
  elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
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
endif()

option(LIB_CHECK_PYINSTALL "Check for library during python install" OFF)
add_subdirectory(src)
add_subdirectory(bindings)
if(BUILD_TESTS_EXAMPLES)
  add_subdirectory(tests)
  add_subdirectory(examples)
endif()

if(MSVC)
  configure_file(libgenalyzer.iss.cmakein ${CMAKE_CURRENT_BINARY_DIR}/libgenalyzer.iss @ONLY)
endif()

install(TARGETS genalyzer DESTINATION /usr/local/lib)
