file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/bindings/c/src/*.cpp")
set(GENALYZER_MAIN_HEADER "${PROJECT_SOURCE_DIR}/bindings/c/include/cgenalyzer.h")

add_definitions(-DGENALYZER_EXPORTS)

add_library(genalyzer SHARED ${SOURCE_LIST} ${HEADER_LIST})

target_link_libraries(genalyzer LINK_PRIVATE swx_analysis_genalyzer)

target_include_directories(genalyzer PUBLIC "${PROJECT_SOURCE_DIR}/bindings/c/include")

set_target_properties(genalyzer PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
  FRAMEWORK TRUE
  PUBLIC_HEADER "${GENALYZER_MAIN_HEADER}"
  LINKER_LANGUAGE CXX
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

if(MSVC)
  target_compile_options(genalyzer PRIVATE /W4 /WX)
else()
  target_compile_options(genalyzer PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MSVC)
  set_target_properties(genalyzer PROPERTIES OUTPUT_NAME libgenalyzer)
endif()

set(INSTALL_LIB_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH "Installation directory for libraries")

set(GENALYZER_PC ${CMAKE_CURRENT_BINARY_DIR}/libgenalyzer.pc)
configure_file(${PROJECT_SOURCE_DIR}/libgenalyzer.pc.cmakein ${GENALYZER_PC} @ONLY)
install(FILES ${GENALYZER_PC} DESTINATION ${INSTALL_LIB_DIR}/pkgconfig)

install(TARGETS genalyzer
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
	RUNTIME DESTINATION bin
	FRAMEWORK DESTINATION lib
	PUBLIC_HEADER DESTINATION include)

include(GNUInstallDirs)


