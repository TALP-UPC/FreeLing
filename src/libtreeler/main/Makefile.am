
#-----------------------------------------------------------------
#--  main program for treeler (training & testing)

AUTOMAKE_OPTIONS = subdir-objects

include $(top_srcdir)/src/automake_options.am

AM_CPPFLAGS = -I$(top_srcdir)/src/libtreeler $(MY_CPPFLAGS)
AM_CXXFLAGS = -I$(top_srcdir)/src/libtreeler $(MY_CXXFLAGS)

bin_PROGRAMS = treeler

treeler_SOURCES = treeler.cc scripts/registry.cc scripts/train.cc scripts/decode.cc scripts/dump.cc scripts/srl.cc
# scripts/dep.cc 

treeler_LDADD = -ltreeler -lz
treeler_LDFLAGS = -L$(top_srcdir)/src/libtreeler
