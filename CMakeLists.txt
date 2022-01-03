cmake_minimum_required(VERSION 3.13.4)
project(
  swx_analysis_genalyzer
  VERSION 0.0.1
  LANGUAGES CXX C)

set(BUILD_SHARED_LIBS
    ON
    CACHE BOOL "Build shared libraries")

list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  include(CTest)

  if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -fext-numeric-literals")
  endif()   

  find_package(Doxygen)
  find_package(Sphinx)
  find_package(Breathe)
  if(Doxygen_FOUND AND Sphinx_FOUND AND Breathe_FOUND)
    add_subdirectory(doc)
  else()
    message(STATUS "Doxygen or Sphinx or Breathe not found, not building docs")
  endif()
endif()

option(LIB_CHECK_PYINSTALL "Check for library during python install" OFF)
add_subdirectory(src)
add_subdirectory(bindings)
add_subdirectory(tests)

install(TARGETS genalyzer DESTINATION /usr/local/lib)
