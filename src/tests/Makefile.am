EXTRA_DIST = common.h main_test.h performanceMon.h test_speed_config.h

AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = unittests test_speed

include $(top_srcdir)/src/automake_options.am

unittests_SOURCES = main_test.cc dates_modules_test.cc numbers_modules_test.cc dictionary_tests.cc np_test.cc splitter_test.cc
unittests_CXXFLAGS = -I$(top_srcdir)/src/include
unittests_LDADD = -lfreeling $(FREELING_DEPS)
unittests_LDFLAGS = -L$(top_srcdir)/src/libfreeling

test_speed_SOURCES = test_speed.cc
test_speed_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/libtreeler
test_speed_LDADD = -lfreeling $(FREELING_DEPS) -lboost_program_options$(MT) -lboost_system$(MT)
test_speed_LDFLAGS = -L$(top_srcdir)/src/libfreeling

