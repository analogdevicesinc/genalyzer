file(GLOB HEADER_LIST CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/include/*.hpp")

configure_file(
  "${PROJECT_SOURCE_DIR}/include/version.h.in"
  "${PROJECT_BINARY_DIR}/include/version.h"
  @ONLY)

add_definitions(-DEXPORT_API)

add_library(genalyzer_plus_plus STATIC
    array_ops.cpp
    code_density.cpp
    enum_map.cpp
    enum_maps.cpp
    expression.cpp
    formatted_data.cpp
    fourier_analysis.cpp
    fourier_analysis_comp_mask.cpp
    fourier_transforms.cpp
    fourier_utilities.cpp
    json.cpp
    manager.cpp
    platform.cpp
    processes.cpp
    utils.cpp
    version.cpp
    waveforms.cpp
    ${HEADER_LIST}
    )

set_property(TARGET genalyzer_plus_plus PROPERTY POSITION_INDEPENDENT_CODE ON)

# We need this directory, and users of our library will need it too
target_include_directories(genalyzer_plus_plus
  PUBLIC ../include
  PRIVATE "${PROJECT_BINARY_DIR}/include")

set_target_properties(genalyzer_plus_plus PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER "${HEADER_LIST}"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

if(APPLE AND GENALYZER_BUILD_FRAMEWORKS)
  set_target_properties(genalyzer_plus_plus PROPERTIES FRAMEWORK TRUE)
endif()

if(APPLE)
  find_package(PkgConfig)
  pkg_check_modules(FFTW3 REQUIRED fftw3)
endif()
find_package(FFTW REQUIRED)
target_link_libraries(genalyzer_plus_plus LINK_PUBLIC ${FFTW_LIBRARIES})
if(APPLE OR WIN32)
  target_include_directories(genalyzer_plus_plus PRIVATE ${FFTW_INCLUDE_DIRS})
endif()

install(TARGETS genalyzer_plus_plus
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
