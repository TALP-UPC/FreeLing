
# Makefile to build the simple examples of FreeLing usage.
# If your installation paths are not /usr/local, call make with:
#
# make FREELINGDIR=/my/freeling/dir BOOSTINCLUDEDIR=/my/boost/include BOOSTLIBDIR=/my/boost/lib


# directories where freeling and boost are installed
FREELINGDIR=/usr/local
BOOSTINCLUDEDIR=/usr/local/include
BOOSTLIBDIR=/usr/local/lib

# paths for compilation commands below
PATHS=-L$(FREELINGDIR)/lib -I$(FREELINGDIR)/include -I$(FREELINGDIR)/include/foma -L$(BOOSTLIBDIR) -I$(BOOSTINCLUDEDIR)

all:	fl1_morph fl2_tagger fl3_chunker fl4_dependences ident coreferences sample kbest_tagger alts analyzer summary

fl1_morph: fl1_morph.cc
	g++ -std=c++0x -o fl1_morph fl1_morph.cc -lfreeling $(PATHS)

fl2_tagger: fl2_tagger.cc
	g++ -std=c++0x -o fl2_tagger fl2_tagger.cc -lfreeling $(PATHS)

fl3_chunker: fl3_chunker.cc
	g++ -std=c++0x -o fl3_chunker fl3_chunker.cc -lfreeling $(PATHS)

fl4_dependences: fl4_dependences.cc
	g++ -std=c++0x -o fl4_dependences fl4_dependences.cc -lfreeling $(PATHS)

ident: ident.cc
	g++ -std=c++0x -o ident ident.cc -lfreeling $(PATHS)

coreferences: coreferences.cc 
	g++ -std=c++0x -o coreferences coreferences.cc -lfreeling $(PATHS)

analyzer: analyzer.cc
	g++ -std=gnu++0x  -o analyzer analyzer.cc -lfreeling -lboost_program_options $(PATHS)

sample: sample.cc
	g++ -std=c++0x -o sample sample.cc -lfreeling $(PATHS)

kbest_tagger: kbest_tagger.cc
	g++ -std=c++0x -o kbest_tagger kbest_tagger.cc -lfreeling $(PATHS)

alts: alts.cc
	g++ -std=c++0x -o alts alts.cc -lfreeling $(PATHS)

summary: summary.cc
	g++ -std=gnu++0x  -o summary summary.cc -lfreeling $(PATHS)


clean:
	rm -f fl1_morph fl2_tagger fl3_chunker fl4_dependences ident coreferences sample kbest_tagger alts analyzer
