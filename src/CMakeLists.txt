file(GLOB HEADER_LIST CONFIGURE_DEPENDS
     "${PROJECT_SOURCE_DIR}/include/genalyzer/*.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(genalyzer phase.c spectrum.c ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(genalyzer PUBLIC ../include)

set_target_properties(genalyzer PROPERTIES
	SOVERSION ${GENALYZER_API_VERSION_MAJOR}
	FRAMEWORK TRUE
	PUBLIC_HEADER ${HEADER_LIST}
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)

find_package(FFTW REQUIRED)
target_link_libraries(genalyzer LINK_PRIVATE ${FFTW_LIBRARIES})

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
