
if (WIN32)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -DYY_NO_UNISTD_H=1 -DNDEBUG=1 -D_WINDOWS=1 -D_USRDLL=1 -DWIN64=1  -D_MSC_VER=1 )
endif()

## -------------------------------------------------
file(GLOB_RECURSE sources_CRF
crfsuite/crf/crf1d_context.c crfsuite/crf/crf1d_encode.c crfsuite/crf/crf1d_feature.c crfsuite/crf/crf1d_model.c crfsuite/crf/crf1d_tag.c crfsuite/crf/crfsuite.c crfsuite/crf/crfsuite_train.c crfsuite/crf/dataset.c crfsuite/crf/dictionary.c crfsuite/crf/holdout.c crfsuite/crf/logging.c crfsuite/crf/params.c crfsuite/crf/quark.c crfsuite/crf/rumavl.c crfsuite/crf/train_arow.c crfsuite/crf/train_averaged_perceptron.c crfsuite/crf/train_l2sgd.c crfsuite/crf/train_lbfgs.c crfsuite/crf/train_passive_aggressive.c crfsuite/cqdb/cqdb.c crfsuite/cqdb/lookup3.c crfsuite/lbfgs/lbfgs.c crfsuite/crfsuite_api.cpp 
)

add_library(crfsuite SHARED ${sources_CRF})

set_target_properties(crfsuite PROPERTIES LINKER_LANGUAGE CXX)
IF (NOT WIN32)
  target_link_libraries(crfsuite m)
endif()
target_include_directories(crfsuite PUBLIC . crfsuite crfsuite/crf crfsuite/cqdb crfsuite/lbfgs) 

install(TARGETS crfsuite DESTINATION lib)
#install(TARGETS crfsuite
#        RUNTIME DESTINATION bin
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib
#        )

install(FILES crfsuite/crfsuite_api.hpp 
        DESTINATION include/crfsuite
        )

 ## -------------------------------------------------
 ## Build crfsuite utilities (e.g. useful to train models)
      
IF (NOT WIN32)
  file(GLOB_RECURSE crfsuite_main_SRC
     crfsuite/frontend/dump.c crfsuite/frontend/iwa.c crfsuite/frontend/learn.c crfsuite/frontend/main.c crfsuite/frontend/option.c crfsuite/frontend/reader.c crfsuite/frontend/tag.c
    )

  add_executable(crfsuite_bin ${crfsuite_main_SRC})
  target_include_directories(crfsuite_bin PRIVATE crfsuite/frontend)
  target_link_libraries(crfsuite_bin crfsuite)
  set_target_properties(crfsuite_bin PROPERTIES OUTPUT_NAME crfsuite)

  install(TARGETS crfsuite_bin
          RUNTIME DESTINATION bin)

endif()
