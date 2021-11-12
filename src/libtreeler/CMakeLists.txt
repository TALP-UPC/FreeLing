
file(GLOB_RECURSE treeler_SRC
treeler/base/token.cc treeler/base/fidx.cc treeler/base/dictionary.cc treeler/util/options.cc treeler/util/char-utils.cc treeler/util/timer.cc treeler/io/io-basic.cc treeler/control/factory-base.cc treeler/dep/part-dep1.cc treeler/dep/parser-projdep1.cc treeler/dep/part-dep2.cc treeler/dep/part-dep2-index.cc treeler/dep/parser-projdep2.cc treeler/dep/pos-symbols.cc treeler/dep/dependency_parser.cc treeler/tag/part-tag.cc treeler/tag/tuple-seq.cc treeler/tag/io-tag.cc treeler/class/class-basic.cc treeler/class/io-class.cc treeler/class/fgen-class.cc treeler/class/model-mc.cc treeler/srl/part-srl0.cc treeler/srl/simple-parser.cc treeler/srl/srl_parser.cc 
)


if (WIN32)
  list(APPEND treeler_SRC treeler/base/windll.cc)
  add_definitions(-DTREELER_EXPORTS=1 -D_CRT_SECURE_NO_WARNINGS=1 -DNDEBUG=1 -D_WINDOWS=1 -DWIN32=1 -DNOMINMAX=1)
  #SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/src/libtreeler)
  #SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/src/libtreeler)
endif()

# Force to always compile with Wall
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -pedantic -O3")
endif()

add_library(treeler ${treeler_SRC})
set_target_properties(treeler PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(treeler ZLIB::ZLIB)
target_include_directories(treeler PUBLIC .) # TODO: Here we are also including 'main'

install(TARGETS treeler DESTINATION lib)
#install(TARGETS treeler
#        RUNTIME DESTINATION bin
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib)

#install(DIRECTORY treeler/ DESTINATION include/treeler/
#        FILES_MATCHING PATTERN "*.h")
#install(DIRECTORY treeler/ DESTINATION include/treeler/
#        FILES_MATCHING PATTERN "*.tcc")

install(FILES treeler/base/windll.h treeler/base/treeler.h treeler/base/exception.h treeler/base/dataset.h treeler/base/example.h treeler/base/fidx.h treeler/base/feature-idx-v0.h treeler/base/feature-vector.h treeler/base/label.h treeler/base/parameters.h treeler/base/base-parameters.h treeler/base/sparse-parameters.h treeler/base/scores.h treeler/base/wf-scores.h treeler/base/oracle-scores.h treeler/base/score-dumper.h treeler/base/basic-sentence.h treeler/base/sentence.h treeler/base/token.h treeler/base/dictionary.h
        DESTINATION include/treeler/base)

install(FILES treeler/class/class-basic.h treeler/class/fgen-class.h treeler/class/io-class.h treeler/class/model-mc.h
        DESTINATION include/treeler/class)
	
install(FILES treeler/control/control.h treeler/control/trainer.h treeler/control/forall.h treeler/control/factory-base.h treeler/control/factory-scores.h treeler/control/factory-dep.h treeler/control/factory-tag.h treeler/control/factory-ioconll.h treeler/control/models.h treeler/control/model-selector.h
        DESTINATION include/treeler/control)

install(FILES treeler/dep/dep.h treeler/dep/dep-symbols.h treeler/dep/pos-symbols.h treeler/dep/dep-tree.h treeler/dep/dep-eval.h treeler/dep/fgen-dep-v1.h treeler/dep/ftemplates-dep1-v1.h treeler/dep/fgen-basic-dep1.h treeler/dep/fgen-basic-dep1.tcc treeler/dep/fgen-basic-dep2.h treeler/dep/fgen-basic-dep2.tcc treeler/dep/fgen-dep-v0.h treeler/dep/fgen-ftemplates-dep1.h treeler/dep/fgen-ftemplates-dep1.tcc treeler/dep/fgen-ftemplates-dep2.h treeler/dep/fgen-ftemplate-types.h treeler/dep/parser-projdep1.h treeler/dep/parser-projdep1.tcc treeler/dep/parser-projdep2.h treeler/dep/parser-projdep2.tcc treeler/dep/part-dep1.h treeler/dep/part-dep2.h treeler/dep/part-dep2-index.h treeler/dep/dependency_parser.h treeler/dep/score-dumper.h
        DESTINATION include/treeler/dep)

install(FILES treeler/io/io-basic.h treeler/io/conllstream.h treeler/io/io-conll.h treeler/io/io-sentence.h treeler/io/io-dep.h treeler/io/io-fvec.h
        DESTINATION include/treeler/io)

install(FILES treeler/algo/decoder.h treeler/algo/learn-utils.h treeler/algo/learn-utils.tcc treeler/algo/perceptron.h treeler/algo/perceptron.tcc treeler/algo/objectives.h treeler/algo/sgd-trainer.h
        DESTINATION include/treeler/algo)

install(FILES treeler/tag/tag-eval.h treeler/tag/fgen-tag.h treeler/tag/ftemplates-tag-v1.h treeler/tag/fgen-ttag.h treeler/tag/io-tag.h treeler/tag/io-ttag.h treeler/tag/part-tag.h treeler/tag/tag.h treeler/tag/tag-symbols.h treeler/tag/tag-seq.h treeler/tag/tuple-seq.h treeler/tag/viterbi.h
        DESTINATION include/treeler/tag)

install(FILES treeler/util/char-utils.h treeler/util/math-utils.h treeler/util/options.h treeler/util/simple-hash-table.h treeler/util/simple-hash-table.tcc treeler/util/timer.h
        DESTINATION include/treeler/util)

install(FILES treeler/srl/srl.h treeler/srl/sentence.h treeler/srl/paths-container.h treeler/srl/part-srl0.h treeler/srl/simple-parser.h treeler/srl/io.h treeler/srl/fgen-srl-v1.h treeler/srl/fgen-srl-types.h treeler/srl/ftemplates-srl.h treeler/srl/fidx-path.h treeler/srl/srl-eval.h treeler/srl/factory-srl.h treeler/srl/paths-defs.h treeler/srl/srl_parser.h treeler/srl/scorers.h
        DESTINATION include/treeler/srl)

# treeler main is used only for training and gives problems in windows.
# If needed, models can be trained on linux and copied to the windows production system      
if (NOT WIN32)
  add_subdirectory(main)
endif()





