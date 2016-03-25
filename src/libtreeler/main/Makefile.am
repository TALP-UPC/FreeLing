
#-----------------------------------------------------------------
#--  main program for treeler (training & testing)

AUTOMAKE_OPTIONS = subdir-objects

AM_CPPFLAGS = -I$(top_srcdir)/src/libtreeler
AM_CXXFLAGS = -I$(top_srcdir)/src/libtreeler

bin_PROGRAMS = treeler

treeler_SOURCES = treeler.cc scripts/registry.cc scripts/train.cc scripts/decode.cc scripts/dump.cc scripts/srl.cc
# scripts/dep.cc 

treeler_LDADD = -ltreeler -lz
treeler_LDFLAGS = -L$(top_srcdir)/src/libtreeler
