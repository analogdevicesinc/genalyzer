file(GLOB HEADER_LIST CONFIGURE_DEPENDS
     "${PROJECT_SOURCE_DIR}/include/genalyzer/*.h")

# Make an automatic library - will be static or dynamic based on user setting
add_library(genalyzer phase.c ${HEADER_LIST})

# We need this directory, and users of our library will need it too
target_include_directories(genalyzer PUBLIC ../include)

# IDEs should put the headers in a nice place
source_group(
  TREE "${PROJECT_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST})
