
# Add include directory to all projects and libraries
include_directories(include)

find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
endif( ZLIB_FOUND )

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

