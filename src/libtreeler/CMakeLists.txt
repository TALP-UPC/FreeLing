
file(GLOB_RECURSE treeler_SRC
treeler/base/token.cc treeler/base/fidx.cc treeler/base/dictionary.cc treeler/util/options.cc treeler/util/char-utils.cc treeler/util/timer.cc treeler/io/io-basic.cc treeler/control/factory-base.cc treeler/dep/part-dep1.cc treeler/dep/parser-projdep1.cc treeler/dep/part-dep2.cc treeler/dep/part-dep2-index.cc treeler/dep/parser-projdep2.cc treeler/dep/pos-symbols.cc treeler/dep/dependency_parser.cc treeler/tag/part-tag.cc treeler/tag/tuple-seq.cc treeler/tag/io-tag.cc treeler/class/class-basic.cc treeler/class/io-class.cc treeler/class/fgen-class.cc treeler/class/model-mc.cc treeler/srl/part-srl0.cc treeler/srl/simple-parser.cc treeler/srl/srl_parser.cc 
)

add_library(treeler ${treeler_SRC})
set_target_properties(treeler PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(treeler PUBLIC .) # TODO: Here we are also including 'main'

install(TARGETS treeler
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)


add_subdirectory(main)



