
include_directories( ${ZLIB_INCLUDE_DIRS} )

if (WIN32)
  add_definitions(-DFOMA_EXPORTS=1 -D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -DYY_NO_UNISTD_H=1 -DNDEBUG=1 -D_WINDOWS=1 -D_USRDLL=1 -DWIN32=1)
endif()

file(GLOB_RECURSE sources
foma/apply.c foma/coaccessible.c foma/constructions.c foma/define.c foma/determinize.c foma/dynarray.c foma/extract.c foma/flags.c foma/int_stack.c foma/io.c foma/lex.cmatrix.c foma/lexcread.c foma/lex.lexc.c foma/lex.yy.c foma/mem.c foma/minimize.c foma/regex.c foma/reverse.c foma/rewrite.c foma/sigma.c foma/spelling.c foma/stringhash.c foma/structures.c foma/topsort.c foma/trie.c foma/utf8.c
)

add_library(foma ${sources})
target_link_libraries(foma ZLIB::ZLIB)
set_target_properties(foma PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(foma PUBLIC .)

install(TARGETS foma DESTINATION lib)
#install(TARGETS foma
#        RUNTIME DESTINATION bin
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib)

install(DIRECTORY foma/ DESTINATION include/foma
        FILES_MATCHING PATTERN "*.h")
