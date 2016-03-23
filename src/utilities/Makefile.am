EXTRA_DIST = coref corrector dicc-management lang_ident train-nerc train-tagger

AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = dicc2phon

include $(top_srcdir)/src/automake_options.am

dicc2phon_SOURCES = corrector/dicc2phon.cc
dicc2phon_CPPFLAGS = $(CPPFLAGS) -I$(top_srcdir)/src/include/
dicc2phon_LDADD = -lfreeling $(FREELING_DEPS)
dicc2phon_LDFLAGS = $(LDFLAGS) -L$(top_srcdir)/src/libfreeling
