
file(GLOB_RECURSE treeler_main_SRC
     treeler.cc scripts/registry.cc scripts/train.cc scripts/decode.cc scripts/dump.cc scripts/srl.cc)

if (WIN32)
  add_definitions(-DTREELER_EXPORTS=1 -D_CRT_SECURE_NO_WARNINGS=1 -DNDEBUG=1 -D_WINDOWS=1 -DWIN32=1 -DNOMINMAX=1)
endif()

add_executable(treeler_bin ${treeler_main_SRC})
target_include_directories(treeler_bin PRIVATE .)
target_link_libraries(treeler_bin treeler ${ZLIB_LIBRARIES})
set_target_properties(treeler_bin PROPERTIES OUTPUT_NAME treeler)

install(TARGETS treeler_bin
        RUNTIME DESTINATION bin)

