
if (WIN32)
  add_definitions(-DFOMA_EXPORTS=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -DYY_NO_UNISTD_H=1 -DNDEBUG=1 -D_WINDOWS=1 -D_USRDLL=1 -DWIN32=1)
endif()

## -------------------------------------------------
file(GLOB_RECURSE sources_CRF
src/crf/crf1d_context.c src/crf/crf1d_encode.c src/crf/crf1d_feature.c src/crf/crf1d_model.c src/crf/crf1d_tag.c src/crf/crfsuite.c src/crf/crfsuite_train.c src/crf/dataset.c src/crf/dictionary.c src/crf/holdout.c src/crf/logging.c src/crf/params.c src/crf/quark.c src/crf/rumavl.c src/crf/train_arow.c src/crf/train_averaged_perceptron.c src/crf/train_l2sgd.c src/crf/train_lbfgs.c src/crf/train_passive_aggressive.c src/cqdb/cqdb.c src/cqdb/lookup3.c src/lbfgs/lbfgs.c include/crfsuite_api.cpp 
)

add_library(crfsuite SHARED ${sources_CRF})
set_target_properties(crfsuite PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(crfsuite m)
target_include_directories(crfsuite PUBLIC include src/crf src/cqdb src/lbfgs) 

install(TARGETS crfsuite
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

install(FILES include/crfsuite_api.hpp 
        DESTINATION include/crfsuite
        )

## -------------------------------------------------
file(GLOB_RECURSE crfsuite_main_SRC
     src/frontend/dump.c src/frontend/iwa.c src/frontend/learn.c src/frontend/main.c src/frontend/option.c src/frontend/reader.c src/frontend/tag.c
    )

add_executable(crfsuite_bin ${crfsuite_main_SRC})
target_include_directories(crfsuite_bin PRIVATE src/frontend)
target_link_libraries(crfsuite_bin crfsuite)
set_target_properties(crfsuite_bin PROPERTIES OUTPUT_NAME crfsuite)

install(TARGETS crfsuite_bin
        RUNTIME DESTINATION bin)

