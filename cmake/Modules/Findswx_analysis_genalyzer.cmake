if(NOT SWX_ANALYSIS_GENALYZER_FOUND)
  find_path(SWX_ANALYSIS_GENALYZER_INCLUDE_DIRS 
    NAMES
    abstract_object.hpp
    analysis.hpp
    array_math.hpp
    checks.hpp
    code_density.hpp
    constants.hpp
    dft.hpp
    enum_map.hpp
    enum_maps.hpp
    enums.hpp
    fft_analysis2.hpp
    fft_component.hpp
    file_utils.hpp   
    formatted_data.hpp
    io_data.hpp
    io_item.hpp
    io_map.hpp
    io_null.hpp
    io_object.hpp
    io_vector.hpp
    math_expression.hpp
    math_expression_tokens.hpp
    pragmas.hpp
    processes.hpp
    real_to_string.hpp
    sparse_array_mask.hpp
    text_utils.hpp
    type_aliases.hpp
    types.hpp
    var_data.hpp
    var_item.hpp
    var_map.hpp
    var_vector.hpp
    waveforms.hpp
    PATHS
    /usr/include/
    /usr/local/include/
  )

  find_library(SWX_ANALYSIS_GENALYZER_LIBRARIES 
    NAMES 
    swx_analysis_genalyzer
    PATHS
    /usr/lib
    /usr/local/lib
  )

if(SWX_ANALYSIS_GENALYZER_INCLUDE_DIRS AND SWX_ANALYSIS_GENALYZER_LIBRARIES)
  set(SWX_ANALYSIS_GENALYZER_FOUND TRUE CACHE INTERNAL "libswx_analysis_genalyzer found")
  message(STATUS "Found libswx_analysis_genalyzer: ${SWX_ANALYSIS_GENALYZER_INCLUDE_DIRS}, ${SWX_ANALYSIS_GENALYZER_LIBRARIES}")
else(SWX_ANALYSIS_GENALYZER_INCLUDE_DIRS AND SWX_ANALYSIS_GENALYZER_LIBRARIES)
  set(SWX_ANALYSIS_GENALYZER_FOUND FALSE CACHE INTERNAL "libswx_analysis_genalyzer found")
  message(STATUS "libswx_analysis_genalyzer not found.")
endif(SWX_ANALYSIS_GENALYZER_INCLUDE_DIRS AND SWX_ANALYSIS_GENALYZER_LIBRARIES)

mark_as_advanced(SWX_ANALYSIS_GENALYZER_LIBRARIES AND SWX_ANALYSIS_GENALYZER_INCLUDE_DIRS)

endif(NOT SWX_ANALYSIS_GENALYZER_FOUND)
