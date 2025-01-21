find_program(BREATHE_APIDOC NAMES breathe-apidoc
    DOC "Breathe Doxygen extension for Sphinx"
)

if(BREATHE_APIDOC)
    set(Breath_FOUND TRUE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(BREATHE REQUIRED_VARS BREATHE_APIDOC
    VERSION_VAR BREATHE_VERSION
)

mark_as_advanced(BREATHE_APIDOC)