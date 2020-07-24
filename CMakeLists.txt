cmake_minimum_required(VERSION 3.12...3.16)
project(
  genalyzer
  VERSION 0.0.1
  DESCRIPTION "Library of DSP functions for RF measurements"
  LANGUAGES C)

# ############################ Dependencies ####################################
# Only do these if this is the main project, and not if it is included through
# add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

  # Let's nicely support folders in IDEs
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  # Testing only available if this is the main app Note this needs to be done in
  # the main CMakeLists since it calls enable_testing, which must be in the main
  # CMakeLists.
  include(CTest)

  # find_package(OpenBLAS REQUIRED)

  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(doc)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()

# ###################### Disable In-source builds ##############################
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(
    FATAL_ERROR
      "In-source builds are not allowed. \
    Consider making a separate build folder and run cmake \
    from there using this command:
    cmake ${CMAKE_SOURCE_DIR}")
endif()

# ############################## Source ########################################
add_subdirectory(src)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND BUILD_TESTING)
  add_subdirectory(tests)
endif()

# ############################## Version #######################################
configure_file(cmake/version.h.cmakein
               ${CMAKE_SOURCE_DIR}/include/genalyzer/version.h @ONLY)
