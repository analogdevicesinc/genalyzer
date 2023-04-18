if(WIN32)
  set(LIBRARIES genalyzer)
else()
  set(LIBRARIES genalyzer m)
endif()

add_executable(do_wfa_sim_example do_wfa_sim_example.c)
target_link_libraries(do_wfa_sim_example ${LIBRARIES})

add_executable(do_ha_sim_example do_ha_sim_example.c)
target_link_libraries(do_ha_sim_example ${LIBRARIES})

add_executable(do_fa_pluto_example do_fa_pluto_example.c)
target_link_libraries(do_fa_pluto_example ${LIBRARIES} cJSON)

add_executable(do_fa_pluto_example_all_results do_fa_pluto_example_all_results.c)
target_link_libraries(do_fa_pluto_example_all_results ${LIBRARIES} cJSON)

add_executable(real_analysis_genalyzer_advanced real_analysis_genalyzer_advanced.c)
target_link_libraries(real_analysis_genalyzer_advanced ${LIBRARIES})

add_executable(fft_analysis_genalyzer_advanced fft_analysis_genalyzer_advanced.c)
target_link_libraries(fft_analysis_genalyzer_advanced ${LIBRARIES})