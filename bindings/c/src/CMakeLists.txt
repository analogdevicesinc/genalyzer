file(GLOB SOURCE_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/bindings/c/src/*.cpp" "${PROJECT_SOURCE_DIR}/bindings/c/src/*.c")
set(GENALYZER_MAIN_HEADER "${PROJECT_SOURCE_DIR}/bindings/c/include/cgenalyzer_simplified_beta.h")

add_definitions(-DGENALYZER_EXPORTS)

add_library(genalyzer SHARED ${SOURCE_LIST} ${HEADER_LIST})

target_link_libraries(genalyzer LINK_PRIVATE genalyzer_plus_plus)

target_include_directories(genalyzer PUBLIC "${PROJECT_SOURCE_DIR}/bindings/c/include")

if(COVERAGE)
  target_compile_options(genalyzer PRIVATE --coverage)
  target_link_libraries(genalyzer PRIVATE --coverage)
endif()

if(UNIX)
  # If GCC is older than 8.0.0, enable experimental filesystem
  execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
  if(GCC_VERSION VERSION_LESS 8.0.0)
    message(STATUS "GCC version < 8.0.0, enabling experimental filesystem")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lstdc++fs")
    target_compile_definitions(genalyzer PUBLIC EXPERIMENTAL_FILESYSTEM)
  endif()
endif()

set_target_properties(genalyzer PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
  PUBLIC_HEADER "${GENALYZER_MAIN_HEADER}"
  LINKER_LANGUAGE CXX
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

if(APPLE)
  set_target_properties(genalyzer PROPERTIES
                                        INSTALL_NAME_DIR
                                        "${CMAKE_INSTALL_FULL_LIBDIR}")
endif()

if(APPLE AND GENALYZER_BUILD_FRAMEWORKS)
  set_target_properties(genalyzer PROPERTIES FRAMEWORK TRUE)
endif()

if(MSVC)
  target_compile_options(genalyzer PRIVATE /W4 /WX)
else()
  target_compile_options(genalyzer PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(MSVC)
  set_target_properties(genalyzer PROPERTIES OUTPUT_NAME libgenalyzer)
endif()

set(GENALYZER_PC ${CMAKE_CURRENT_BINARY_DIR}/libgenalyzer.pc)
configure_file(${PROJECT_SOURCE_DIR}/libgenalyzer.pc.cmakein ${GENALYZER_PC} @ONLY)
install(FILES ${GENALYZER_PC}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

install(TARGETS genalyzer
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  FRAMEWORK DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


