
# Add include directory to all projects and libraries
include_directories(include)

# disable autolinking in boost, force dynamic linking for boost,
# deactivate deprecated stuff that forces boost_system
add_definitions( -DBOOST_ALL_NO_LIB -DBOOST_ALL_DYN_LINK )

set(EIGEN3_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/src/eigen3)
add_subdirectory(libdynet)

add_subdirectory(libfoma)
add_subdirectory(libtreeler)
add_subdirectory(libfreeling)
add_subdirectory(main)
add_subdirectory(utilities)
if(BUILD_TESTS)
   add_subdirectory(tests)
endif()

install(DIRECTORY include/ DESTINATION include
        FILES_MATCHING PATTERN "*.h"
        PATTERN "coref.h" EXCLUDE
        PATTERN "coref_fex.h" EXCLUDE)

install(DIRECTORY eigen3 DESTINATION include)

