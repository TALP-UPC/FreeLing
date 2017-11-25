
file(GLOB_RECURSE treeler_main_SRC
treeler.cc scripts/registry.cc scripts/train.cc scripts/decode.cc scripts/dump.cc scripts/srl.cc
)

find_package( ZLIB REQUIRED )
if ( ZLIB_FOUND )
    include_directories( ${ZLIB_INCLUDE_DIRS} )
endif( ZLIB_FOUND )

add_executable(treeler_bin ${treeler_main_SRC})
target_include_directories(treeler_bin PRIVATE .)
target_link_libraries(treeler_bin treeler ${ZLIB_LIBRARIES})

install(TARGETS treeler_bin
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

