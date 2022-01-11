file(GLOB_RECURSE GENALYZER_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/include/*.h)

set(DOXYGEN_INPUT_DIR ${PROJECT_SOURCE_DIR}/bindings/c/include)
set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/out_docs)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)

file(MAKE_DIRECTORY ${DOXYGEN_OUTPUT_DIR})
add_custom_command(
  OUTPUT ${DOXYGEN_INDEX_FILE}
  DEPENDS ${GENALYZER_PUBLIC_HEADERS}
  COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
  MAIN_DEPENDENCY ${DOXYFILE_OUT}
  ${DOXYFILE_IN}
  COMMENT "Generating docs")

add_custom_target(Doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})

# DOXYBOOK2 BINARY #######
if(WIN32)
  set(URL
      "https://github.com/matusnovak/doxybook2/releases/download/v1.4.0/doxybook2-windows-win64-v1.4.0.zip"
  )
  set(HASH "42f537a7b2b4d3f54b65211f856b8f95a9b7227191d9ba70fa19484dfe5f6f07")
else()
  set(URL
      "https://github.com/matusnovak/doxybook2/releases/download/v1.4.0/doxybook2-linux-amd64-v1.4.0.zip"
  )
  set(HASH "bab9356f5daa550cbf21d8d9b554ea59c8be039716a2caf6e96dee52713fccb0")
endif()
include(FetchContent)
FetchContent_Declare(
  doxybook2binary
  URL ${URL}
  URL_HASH SHA256=${HASH})
FetchContent_MakeAvailable(doxybook2binary)
set(DOXYBOOK2_EXECUTABLE
    ${CMAKE_CURRENT_BINARY_DIR}/../_deps/doxybook2binary-src/bin/doxybook2)

# DOXYBOOK2 #######
set(DOXYBOOK2_BUILD ${DOXYGEN_OUTPUT_DIR}/doxybook2)
file(MAKE_DIRECTORY ${DOXYBOOK2_BUILD})

set(DOXYBOOK2_BUILD ${DOXYGEN_OUTPUT_DIR}/doxybook2)
set(DOXYBOOK2_INDEX_FILE ${DOXYBOOK2_BUILD}/Files/index.md)

add_custom_command(
  OUTPUT ${DOXYBOOK2_INDEX_FILE}
  COMMAND
    ${DOXYBOOK2_EXECUTABLE} --input ${DOXYGEN_OUTPUT_DIR}/xml --output
    ${DOXYBOOK2_BUILD} --config ${PROJECT_SOURCE_DIR}/docs/doxybook2config.json
    --templates ${PROJECT_SOURCE_DIR}/docs/_templates
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS # Other docs files you want to track should go here (or in some
          # variable)
  MAIN_DEPENDENCY ${DOXYGEN_INDEX_FILE}
  COMMENT "Generating documentation with doxybook2")

add_custom_target(Doxybook2 ALL DEPENDS ${DOXYBOOK2_INDEX_FILE})

# PREP-MKDOCS #######
set(MKDOCS_BUILD ${DOXYGEN_OUTPUT_DIR}/mkdocs)
file(MAKE_DIRECTORY ${MKDOCS_BUILD})
set(DOXYBOOK2_OUT_FILE ${MKDOCS_BUILD}/docs/cgenalyzer_8h.md)

# Get docs folder
file(COPY ${PROJECT_SOURCE_DIR}/docs DESTINATION ${MKDOCS_BUILD})
# Get config file
file(COPY ${PROJECT_SOURCE_DIR}/mkdocs.yml DESTINATION ${MKDOCS_BUILD})
# Copy dependent doxybook2 output
add_custom_command(
  OUTPUT ${DOXYBOOK2_OUT_FILE}
  COMMAND ${CMAKE_COMMAND} -E copy ${DOXYBOOK2_BUILD}/Files/cgenalyzer_8h.md
          ${MKDOCS_BUILD}/docs/cgenalyzer_8h.md
  DEPENDS # Other docs files you want to track should go here (or in some
          # variable)
  MAIN_DEPENDENCY ${DOXYBOOK2_INDEX_FILE})
add_custom_target(PostDoxybook2 ALL DEPENDS ${DOXYBOOK2_OUT_FILE})

# MKDOCS #######

set(MKDOCS_EXECUTABLE mkdocs)
set(MKDOCS_INDEX_FILE ${MKDOCS_BUILD}/out/index.html)

add_custom_command(
  OUTPUT ${MKDOCS_INDEX_FILE}
  COMMAND ${MKDOCS_EXECUTABLE} build -f ${MKDOCS_BUILD}/mkdocs.yml -d
          ${MKDOCS_BUILD}/out/
  WORKING_DIRECTORY ${MKDOCS_BUILD}
  DEPENDS # Other docs files you want to track should go here (or in some
          # variable)
  MAIN_DEPENDENCY ${DOXYBOOK2_OUT_FILE}
  COMMENT "Generating documentation with mkdocs")

add_custom_target(MkDocs ALL DEPENDS ${MKDOCS_INDEX_FILE})

include(GNUInstallDirs)
install(DIRECTORY ${MKDOCS_BUILD}/out DESTINATION ${CMAKE_INSTALL_DOCDIR})
