
# Basic test for CI

include(CTest)

add_executable(fl_test test.cc)
if(WIN32)
  target_link_libraries(fl_test freeling wsock32 ws2_32)
else()
  target_link_libraries(fl_test freeling ${CMAKE_THREAD_LIBS_INIT})  
endif()   


