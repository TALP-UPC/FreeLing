
AUTOMAKE_OPTIONS = subdir-objects

include $(top_srcdir)/src/automake_options.am

AM_CPPFLAGS = $(KALDI_INCLUDES) $(KALDI_DEFINES) -I$(top_srcdir)/src/include
AM_CXXFLAGS = -Wno-unused-local-typedefs

lib_LTLIBRARIES = libasr.la

libasr_la_SOURCES = asr/asr.cc asr/base64.cc asr/Nnet3LatgenFasterDecoder.cc asr/OnlineDecoder.cc asr/Request.cc asr/Response.cc

librarydir=$(includedir)
nobase_library_HEADERS = asr/asr.h asr/base64.h asr/Decoder.h asr/Nnet3LatgenFasterDecoder.h asr/OnlineDecoder.h asr/Request.h asr/Response.h

libasr_la_LIBADD = $(KALDI_DEPS) -lfreeling

libasr_la_LDFLAGS=-module -avoid-version -shared $(KALDI_LIBS)
