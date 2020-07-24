add_executable(ComplexPhaseEstTest complex_phase_test.c)
if(WIN32)
  target_link_libraries(ComplexPhaseEstTest LINK_PRIVATE ${LIBIIO_LIBRARIES} genalyzer)
else()
  target_link_libraries(ComplexPhaseEstTest LINK_PRIVATE ${LIBIIO_LIBRARIES} genalyzer m)
endif()
add_test(NAME ComplexPhaseEstTest
         COMMAND ComplexPhaseEstTest
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
