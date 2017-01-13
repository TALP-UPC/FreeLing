EXTRA_DIST = omlet sample_analyzer simple_examples README

AUTOMAKE_OPTIONS = subdir-objects

include $(top_srcdir)/src/automake_options.am
AM_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/libtreeler -I$(top_srcdir)/src/libfoma $(KALDI_INCLUDES) $(MY_CPPFLAGS)
AM_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/libtreeler -I$(top_srcdir)/src/libfoma $(KALDI_INCLUDES) $(MY_CXXFLAGS)

bin_PROGRAMS = analyzer analyzer_client threaded_analyzer


# threaded analyzer sample program
threaded_analyzer_SOURCES = sample_analyzer/threaded_analyzer.cc sample_analyzer/config.h sample_analyzer/output.h  sample_analyzer/threaded_processor.h
threaded_analyzer_LDADD = -lfreeling $(FREELING_DEPS) -lboost_program_options$(MT) -lboost_thread$(MT) 
threaded_analyzer_LDFLAGS = -L$(top_srcdir)/src/libfreeling $(KALDI_LIBS)

# analyzer sample program
analyzer_SOURCES = sample_analyzer/main.cc sample_analyzer/config.h sample_analyzer/output.h sample_analyzer/socket.h sample_analyzer/stats.h
analyzer_LDADD = -lfreeling $(FREELING_DEPS) -lboost_program_options$(MT) -lpthread
analyzer_LDFLAGS = -L$(top_srcdir)/src/libfreeling $(KALDI_LIBS)

# sample client for analyzer in server mode
analyzer_client_SOURCES = sample_analyzer/analyzer_client.cc sample_analyzer/socket.h

install-exec-hook:
	cp sample_analyzer/analyze sample_analyzer/fl_initialize $(DESTDIR)/$(bindir)
	chmod +x $(DESTDIR)/$(bindir)/analyze

uninstall-hook:
	@rm -rf $(DESTDIR)/$(bindir)/analyze $(DESTDIR)/$(bindir)/fl_initialize
