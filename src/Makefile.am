
if BUILD_TESTS
  DO_TESTS = tests
endif

if NO_LIBFOMA
  DO_FOMA =
else
  DO_FOMA = libfoma
endif

SUBDIRS = include libtreeler $(DO_FOMA) libfreeling main utilities $(DO_TESTS)
