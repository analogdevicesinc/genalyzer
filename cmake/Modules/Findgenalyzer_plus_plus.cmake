if(NOT GENALYZER_PLUS_PLUS_FOUND)
  find_path(GENALYZER_PLUS_PLUS_INCLUDE_DIRS 
    NAMES
    array_ops.hpp
    code_density.hpp
    constants.hpp
    enum_map.hpp
    enum_maps.hpp
    enums.hpp
    exceptions.hpp
    expression.hpp
    formatted_data.hpp
    fourier_analysis_comp_mask.hpp
    fourier_analysis_component.hpp
    fourier_analysis_results.hpp
    fourier_analysis.hpp
    fourier_transforms.hpp
    fourier_utilities.hpp
    json.hpp
    manager.hpp
    object.hpp
    processes.hpp
    reductions.hpp
    type_aliases.hpp
    utils.hpp
    version.hpp
    waveforms.hpp
    PATHS
    /usr/include/
    /usr/local/include/
  )

  find_library(GENALYZER_PLUS_PLUS_LIBRARIES 
    NAMES 
    genalyzer_plus_plus
    PATHS
    /usr/lib
    /usr/local/lib
  )

if(GENALYZER_PLUS_PLUS_INCLUDE_DIRS AND GENALYZER_PLUS_PLUS_LIBRARIES)
  set(GENALYZER_PLUS_PLUS_FOUND TRUE CACHE INTERNAL "libgenalyzer_plus_plus found")
  message(STATUS "Found libgenalyzer_plus_plus: ${GENALYZER_PLUS_PLUS_INCLUDE_DIRS}, ${GENALYZER_PLUS_PLUS_LIBRARIES}")
else(GENALYZER_PLUS_PLUS_INCLUDE_DIRS AND GENALYZER_PLUS_PLUS_LIBRARIES)
  set(GENALYZER_PLUS_PLUS_FOUND FALSE CACHE INTERNAL "libgenalyzer_plus_plus found")
  message(STATUS "libgenalyzer_plus_plus not found.")
endif(GENALYZER_PLUS_PLUS_INCLUDE_DIRS AND GENALYZER_PLUS_PLUS_LIBRARIES)

mark_as_advanced(GENALYZER_PLUS_PLUS_LIBRARIES AND GENALYZER_PLUS_PLUS_INCLUDE_DIRS)

endif(NOT GENALYZER_PLUS_PLUS_FOUND)
