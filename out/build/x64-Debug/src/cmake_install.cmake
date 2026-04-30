# Install script for directory: C:/git/genalyzer/src

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/git/genalyzer/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/git/genalyzer/out/build/x64-Debug/src/genalyzer_plus_plus.lib")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/git/genalyzer/include/array_ops.hpp"
    "C:/git/genalyzer/include/code_density.hpp"
    "C:/git/genalyzer/include/constants.hpp"
    "C:/git/genalyzer/include/enum_map.hpp"
    "C:/git/genalyzer/include/enum_maps.hpp"
    "C:/git/genalyzer/include/enums.hpp"
    "C:/git/genalyzer/include/exceptions.hpp"
    "C:/git/genalyzer/include/expression.hpp"
    "C:/git/genalyzer/include/formatted_data.hpp"
    "C:/git/genalyzer/include/fourier_analysis.hpp"
    "C:/git/genalyzer/include/fourier_analysis_comp_mask.hpp"
    "C:/git/genalyzer/include/fourier_analysis_component.hpp"
    "C:/git/genalyzer/include/fourier_analysis_results.hpp"
    "C:/git/genalyzer/include/fourier_transforms.hpp"
    "C:/git/genalyzer/include/fourier_utilities.hpp"
    "C:/git/genalyzer/include/json.hpp"
    "C:/git/genalyzer/include/manager.hpp"
    "C:/git/genalyzer/include/object.hpp"
    "C:/git/genalyzer/include/processes.hpp"
    "C:/git/genalyzer/include/reductions.hpp"
    "C:/git/genalyzer/include/type_aliases.hpp"
    "C:/git/genalyzer/include/utils.hpp"
    "C:/git/genalyzer/include/version.hpp"
    "C:/git/genalyzer/include/waveforms.hpp"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/git/genalyzer/out/build/x64-Debug/src/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
