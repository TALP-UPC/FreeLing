
file(GLOB_RECURSE treeler_main_SRC
treeler.cc scripts/registry.cc scripts/train.cc scripts/decode.cc scripts/dump.cc scripts/srl.cc
)

add_executable(treeler_bin ${treeler_main_SRC})
target_include_directories(treeler_bin PRIVATE .)
target_link_libraries(treeler_bin treeler)

