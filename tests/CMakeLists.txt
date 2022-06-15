enable_testing()

if(WIN32)
  set(LIBRARIES genalyzer)
else()
  set(LIBRARIES genalyzer m)
endif()

################################################################################
file(GLOB TEST_FILES_LIST "test_vectors/test_gen_tone_*.json")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_gen_tone.c PROPERTIES LANGUAGE C)
add_executable(test_gen_tone test_gen_tone.c test_genalyzer.h)
target_link_libraries(test_gen_tone ${LIBRARIES})
foreach(test_file ${TEST_FILES_LIST})
  add_test(NAME test_gen_tone_${n}
    COMMAND test_gen_tone ${test_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

if(FALSE)
################################################################################
file(GLOB TEST_FILES_LIST "test_vectors/test_gen_ramp_[^and_quantize_]*.txt")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_gen_ramp.c PROPERTIES LANGUAGE C)
add_executable(test_gen_ramp test_gen_ramp.c test_genalyzer.h)
target_link_libraries(test_gen_ramp ${LIBRARIES})
foreach(test_file ${TEST_FILES_LIST})
  add_test(NAME test_gen_ramp_${n}
    COMMAND test_gen_ramp ${test_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

################################################################################
file(GLOB TEST_FILES_LIST "test_vectors/test_gen_ramp_and_quantize_*.txt")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_gen_ramp_and_quantize.c PROPERTIES LANGUAGE C)
add_executable(test_gen_ramp_and_quantize test_gen_ramp_and_quantize.c test_genalyzer.h)
target_link_libraries(test_gen_ramp_and_quantize ${LIBRARIES})
foreach(test_file ${TEST_FILES_LIST})
  add_test(NAME test_gen_ramp_and_quantize_${n}
    COMMAND test_gen_ramp_and_quantize ${test_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

################################################################################
file(GLOB TEST_FILES_LIST "test_vectors/test_quantize_*.txt")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_quantize.c PROPERTIES LANGUAGE C)
add_executable(test_quantize test_quantize.c test_genalyzer.h)
target_link_libraries(test_quantize ${LIBRARIES})
foreach(test_file ${TEST_FILES_LIST})
  add_test(NAME test_quantize_${n}
    COMMAND test_quantize ${test_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

################################################################################
file(GLOB TEST_IN_FILES_LIST "test_vectors/test_rfft_input_*.txt")
file(GLOB TEST_OUT_FILES_LIST "test_vectors/test_rfft_output_*.txt")
list(LENGTH TEST_IN_FILES_LIST IN_LENGTH)
math(EXPR IN_LENGTH "${IN_LENGTH} - 1")
SET_SOURCE_FILES_PROPERTIES(test_rfft.c PROPERTIES LANGUAGE C)
add_executable(test_rfft test_rfft.c test_genalyzer.h)
target_link_libraries(test_rfft ${LIBRARIES})
foreach(index RANGE ${IN_LENGTH})
  list(GET TEST_IN_FILES_LIST ${index} IN)
  list(GET TEST_OUT_FILES_LIST ${index} OUT)
  add_test(NAME test_rfft_${index}
    COMMAND test_rfft ${IN} ${OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

################################################################################
file(GLOB TEST_IN_FILES_LIST "test_vectors/test_fft_input_*.txt")
file(GLOB TEST_OUT_FILES_LIST "test_vectors/test_fft_output_*.txt")
list(LENGTH TEST_IN_FILES_LIST IN_LENGTH)
math(EXPR IN_LENGTH "${IN_LENGTH} - 1")
SET_SOURCE_FILES_PROPERTIES(test_fft.c PROPERTIES LANGUAGE C)
add_executable(test_fft test_fft.c test_genalyzer.h)
target_link_libraries(test_fft ${LIBRARIES})
foreach(index RANGE ${IN_LENGTH})
  list(GET TEST_IN_FILES_LIST ${index} IN)
  list(GET TEST_OUT_FILES_LIST ${index} OUT)
  add_test(NAME test_fft_${index}
    COMMAND test_fft ${IN} ${OUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

################################################################################
file(GLOB TEST_FILES_LIST "test_vectors/test_rfft_[^input_][^output_]*.txt")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_metric.c PROPERTIES LANGUAGE C)
add_executable(test_metric test_metric.c test_genalyzer.h)
target_link_libraries(test_metric ${LIBRARIES})
foreach(test_file ${TEST_FILES_LIST})
  add_test(NAME test_metric_${n}
    COMMAND test_metric ${test_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

################################################################################
file(GLOB TEST_FILES_LIST "test_vectors/test_Pluto_OneToneFFT*.txt")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_pluto_metrics.c PROPERTIES LANGUAGE C)
add_executable(test_pluto_metrics test_pluto_metrics.c test_genalyzer.h)
target_link_libraries(test_pluto_metrics ${LIBRARIES})
foreach(test_file ${TEST_FILES_LIST})
  add_test(NAME test_pluto_metrics_${n}
    COMMAND test_pluto_metrics ${test_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

file(GLOB TEST_FILES_LIST "test_vectors/test_quantize64_*.txt")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_quantize64.c PROPERTIES LANGUAGE C)
add_executable(test_quantize64 test_quantize64.c test_genalyzer.h)
target_link_libraries(test_quantize64 ${LIBRARIES})
foreach(test_file ${TEST_FILES_LIST})
  add_test(NAME test_quantize64_${n}
    COMMAND test_quantize64 ${test_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

file(GLOB TEST_IN_FILES_LIST "test_vectors/test_dnl_ramp_input_*.txt")
file(GLOB TEST_OUT_FILES_LIST0 "test_vectors/test_dnl_ramp_hits_output_*.txt")
file(GLOB TEST_OUT_FILES_LIST1 "test_vectors/test_dnl_ramp_dnldata_output_*.txt")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_ramp_dnl.c PROPERTIES LANGUAGE C)
add_executable(test_ramp_dnl test_ramp_dnl.c test_genalyzer.h)
target_link_libraries(test_ramp_dnl
  ${PROJECT_SOURCE_DIR}/build/wrapper_src/libgenalyzer.so
  ${SWX_ANALYSIS_GENALYZER_LIBRARIES} m)
foreach(test_in_file test_out_file0 test_out_file1 IN ZIP_LISTS TEST_IN_FILES_LIST TEST_OUT_FILES_LIST0 TEST_OUT_FILES_LIST1)
  add_test(NAME test_ramp_dnl_${n}
    COMMAND test_ramp_dnl ${test_in_file} ${test_out_file0} ${test_out_file1}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()

file(GLOB TEST_FILES_LIST "test_vectors/test_AD9084Sim_input_*.txt")
set(n 0)
SET_SOURCE_FILES_PROPERTIES(test_AD9084Sim_SFDR.c PROPERTIES LANGUAGE C)
add_executable(test_AD9084Sim_SFDR test_AD9084Sim_SFDR.c test_genalyzer.h)
target_link_libraries(test_AD9084Sim_SFDR
  ${PROJECT_SOURCE_DIR}/build/wrapper_src/libgenalyzer.so
  ${SWX_ANALYSIS_GENALYZER_LIBRARIES} m)
foreach(test_file ${TEST_FILES_LIST})
  add_test(NAME test_AD9084Sim_SFDR_${n}
    COMMAND test_AD9084Sim_SFDR ${test_file}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  math(EXPR n "${n} + 1")
endforeach()
endif()
