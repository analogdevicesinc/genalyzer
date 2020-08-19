add_executable(ComplexPhaseEstTest complex_phase_test.c)
if(WIN32)
  target_link_libraries(ComplexPhaseEstTest LINK_PRIVATE genalyzer)
else()
  target_link_libraries(ComplexPhaseEstTest LINK_PRIVATE genalyzer m)
endif()
add_test(NAME ComplexPhaseEstTest
         COMMAND ComplexPhaseEstTest
         WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

 add_executable(SFDRTest sfdr_test.c)
 if(WIN32)
   target_link_libraries(SFDRTest LINK_PRIVATE genalyzer)
 else()
   target_link_libraries(SFDRTest LINK_PRIVATE genalyzer m)
 endif()
 add_test(NAME SFDRTest
          COMMAND SFDRTest
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

 add_executable(THDTest thd_test.c)
 if(WIN32)
   target_link_libraries(THDTest LINK_PRIVATE genalyzer)
 else()
   target_link_libraries(THDTest LINK_PRIVATE genalyzer m)
 endif()
 add_test(NAME THDTest
          COMMAND THDTest
          WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
