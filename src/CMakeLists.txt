file(GLOB HEADER_LIST CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/include/*.hpp")

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
target_include_directories(genalyzer_plus_plus PUBLIC ../include)

set_target_properties(genalyzer_plus_plus PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
  FRAMEWORK TRUE
  PUBLIC_HEADER "${HEADER_LIST}"
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib"
  CACHE PATH "Installation directory for libraries")

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
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  RUNTIME DESTINATION bin
  FRAMEWORK DESTINATION lib
  PUBLIC_HEADER DESTINATION include)

include(GNUInstallDirs)
