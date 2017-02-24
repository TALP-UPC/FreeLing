
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
LIBS=-lfreeling -lboost_system

all:	fl1_morph fl2_tagger fl3_chunker fl4_dependences ident coreferences sample kbest_tagger alts analyzer summary word_vec corrector

fl1_morph: fl1_morph.cc
	g++ -std=c++0x -o fl1_morph fl1_morph.cc $(PATHS) $(LIBS)

fl2_tagger: fl2_tagger.cc
	g++ -std=c++0x -o fl2_tagger fl2_tagger.cc $(PATHS) $(LIBS)

fl3_chunker: fl3_chunker.cc
	g++ -std=c++0x -o fl3_chunker fl3_chunker.cc $(PATHS) $(LIBS)

fl4_dependences: fl4_dependences.cc
	g++ -std=c++0x -o fl4_dependences fl4_dependences.cc $(PATHS) $(LIBS)

ident: ident.cc
	g++ -std=c++0x -o ident ident.cc $(PATHS) $(LIBS)

coreferences: coreferences.cc 
	g++ -std=c++0x -o coreferences coreferences.cc $(PATHS) $(LIBS)

analyzer: analyzer.cc
	g++ -std=gnu++0x  -o analyzer analyzer.cc  $(PATHS) $(LIBS) -lboost_program_options

sample: sample.cc
	g++ -std=c++0x -o sample sample.cc $(PATHS) $(LIBS)

kbest_tagger: kbest_tagger.cc
	g++ -std=c++0x -o kbest_tagger kbest_tagger.cc $(PATHS) $(LIBS)

alts: alts.cc
	g++ -std=c++0x -o alts alts.cc $(PATHS) $(LIBS)

summary: summary.cc
	g++ -std=gnu++0x  -o summary summary.cc $(PATHS) $(LIBS)

word_vec: word_vec.cc
	g++ -std=gnu++0x  -o word_vec word_vec.cc $(PATHS) $(LIBS)

corrector: corrector.cc
	g++ -std=gnu++0x  -o corrector corrector.cc $(PATHS) $(LIBS)

clean:
	rm -f fl1_morph fl2_tagger fl3_chunker fl4_dependences ident coreferences sample kbest_tagger alts analyzer summary word_vec corrector
