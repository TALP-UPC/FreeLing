
AUTOMAKE_OPTIONS = subdir-objects

include $(top_srcdir)/src/automake_options.am

AM_CPPFLAGS = -I$(top_srcdir)/src/libfoma $(MY_CPPFLAGS)
AM_CXXFLAGS = -I$(top_srcdir)/src/libfoma $(MY_CXXFLAGS)

lib_LTLIBRARIES = libfoma.la

libfoma_la_SOURCES = foma/apply.c foma/coaccessible.c foma/constructions.c foma/define.c foma/determinize.c foma/dynarray.c foma/extract.c foma/flags.c foma/int_stack.c foma/io.c foma/lex.cmatrix.c foma/lexcread.c foma/lex.lexc.c foma/lex.yy.c foma/mem.c foma/minimize.c foma/regex.c foma/reverse.c foma/rewrite.c foma/sigma.c foma/spelling.c foma/stringhash.c foma/structures.c foma/topsort.c foma/trie.c foma/utf8.c

librarydir=$(includedir)
nobase_library_HEADERS = foma/foma.h foma/fomalibconf.h foma/fomalib.h foma/lexc.h foma/regex.h

libfoma_la_LIBADD = -lz
libfoma_la_LDFLAGS = -release 0.9.18

