

# Unittests
include_directories(${Boost_INCLUDE_DIRS})
  find_package(Boost COMPONENTS regex system filesystem thread program_options  unit_test_framework REQUIRED)

add_executable(unittests main_test.cc dates_modules_test.cc numbers_modules_test.cc dictionary_tests.cc np_test.cc splitter_test.cc)
target_link_libraries (unittests freeling ${Boost_LIBRARIES})

# Test speed
add_executable(test_speed test_speed.cc)
target_link_libraries(test_speed freeling ${Boost_LIBRARIES})

#Setup CMake to run tests
enable_testing()
add_test(NAME unittests COMMAND unittests)

