
# Dependencies
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS program_options regex system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIR})


# Analyzer
add_executable(analyzer sample_analyzer/main.cc sample_analyzer/config.h sample_analyzer/socket.h)
target_link_libraries(analyzer freeling ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# Threaded analyzer
add_executable(threaded_analyzer sample_analyzer/threaded_analyzer.cc sample_analyzer/config.h)
target_link_libraries(threaded_analyzer freeling ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# Analyzer client
add_executable(analyzer_client sample_analyzer/analyzer_client.cc sample_analyzer/socket.h)
target_link_libraries(analyzer_client freeling ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# Install targets
install(PROGRAMS sample_analyzer/analyze DESTINATION bin)
install(FILES sample_analyzer/fl_initialize DESTINATION bin)
install(TARGETS analyzer threaded_analyzer
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)


