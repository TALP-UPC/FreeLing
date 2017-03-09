
AUTOMAKE_OPTIONS = subdir-objects

include $(top_srcdir)/src/automake_options.am

AM_CPPFLAGS = -I$(top_srcdir)/src/libasr -I$(top_srcdir)/src/include -DHAVE_ATLAS -DHAVE_OPENFST_GE_10400 -DHAVE_CXXABI_H -DKALDI_DOUBLEPRECISION=0 $(MY_CPPFLAGS)
AM_CXXFLAGS = $(MY_CXXFLAGS)
AM_LDFLAGS = $(KALDI_LIBS)

lib_LTLIBRARIES = libasr.la

libasr_la_SOURCES = asr/asr.cc asr/base64.cc asr/Nnet3LatgenFasterDecoder.cc asr/OnlineDecoder.cc asr/Request.cc asr/Response.cc

librarydir=$(includedir)
nobase_library_HEADERS = asr/asr.h asr/base64.h asr/Decoder.h asr/Nnet3LatgenFasterDecoder.h asr/OnlineDecoder.h asr/Request.h asr/Response.h

#libasr_la_LIBADD = $(KALDI_DEPS)
libasr_la_LIBADD = -lkaldi-base -lkaldi-feat -lkaldi-util -lfst -lkaldi-lat -lkaldi-hmm -lkaldi-online2 -lkaldi-ivector -lkaldi-nnet3 -lkaldi-cudamatrix -lkaldi-matrix -lkaldi-decoder -lkaldi-fstext

libasr_la_LDFLAGS = -release 0.1 

