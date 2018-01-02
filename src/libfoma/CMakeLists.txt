

file(GLOB_RECURSE sources
foma/apply.c foma/coaccessible.c foma/constructions.c foma/define.c foma/determinize.c foma/dynarray.c foma/extract.c foma/flags.c foma/int_stack.c foma/io.c foma/lex.cmatrix.c foma/lexcread.c foma/lex.lexc.c foma/lex.yy.c foma/mem.c foma/minimize.c foma/regex.c foma/reverse.c foma/rewrite.c foma/sigma.c foma/spelling.c foma/stringhash.c foma/structures.c foma/topsort.c foma/trie.c foma/utf8.c
)

add_library(foma ${sources})
target_link_libraries(foma ${ZLIB_LIBRARIES})
set_target_properties(foma PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(foma PUBLIC .)

install(TARGETS foma
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

install(DIRECTORY foma/ DESTINATION include/foma
        FILES_MATCHING PATTERN "*.h")
