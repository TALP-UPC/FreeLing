
# Add include directory to all projects and libraries
include_directories(include)

add_subdirectory(libfoma)
add_subdirectory(libtreeler)
add_subdirectory(libfreeling)

install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.h")

