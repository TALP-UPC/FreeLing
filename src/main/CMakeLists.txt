
# Dependencies
include_directories(${Boost_INCLUDE_DIR})
include_directories(${ICU_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})

if (WIN32)
  # compiler defintions for windows
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -DNOMINMAX -DNDEBUG=1 -DWIN32=1 -D_CONSOLE=1)
endif()

# Analyzer
add_executable(analyzer sample_analyzer/main.cc sample_analyzer/config.h sample_analyzer/socket.h)
target_link_libraries(analyzer freeling)
if(WIN32)
  target_link_libraries(analyzer wsock32 ws2_32)
else()
  target_link_libraries(analyzer ${CMAKE_THREAD_LIBS_INIT})  
endif()

# Analyzer client
add_executable(analyzer_client sample_analyzer/analyzer_client.cc sample_analyzer/socket.h)
if(WIN32)
  target_link_libraries(analyzer_client wsock32 ws2_32)
endif()

# Threaded analyzer
add_executable(threaded_analyzer sample_analyzer/threaded_analyzer.cc sample_analyzer/config.h)
target_link_libraries(threaded_analyzer freeling)


# Install targets
install(PROGRAMS sample_analyzer/analyze DESTINATION bin)
install(FILES sample_analyzer/fl_initialize DESTINATION bin)
install(TARGETS analyzer threaded_analyzer analyzer_client
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)


