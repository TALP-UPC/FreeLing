
# Makefile to build the simple examples of FreeLing usage.
# If your installation paths are not /usr/local, call make with:
#
# make FREELINGDIR=/my/freeling/dir BOOSTINCLUDEDIR=/my/boost/include BOOSTLIBDIR=/my/boost/lib KALDIDIR=/my/kaldi-root/dir

# directories where freeling and boost are installed
FREELINGDIR=/usr/local
BOOSTINCLUDEDIR=/usr/local/include
BOOSTLIBDIR=/usr/local/lib

# paths for compilation commands below
PATHS=-L$(FREELINGDIR)/lib -I$(FREELINGDIR)/include -I$(FREELINGDIR)/include/foma -L$(BOOSTLIBDIR) -I$(BOOSTINCLUDEDIR)
LIBS=-lfreeling -lboost_system

# paths for "asr.cc" example (requires kaldi installed and FreeLing compiled with --with-kaldi-asr option)
KALDIDIR=none
KALDI=-I$(KALDIDIR)/src -I$(KALDIDIR)/tools/ATLAS/include -I$(KALDIDIR)/tools/openfst/include -L$(KALDIDIR)/src/lib -L$(KALDIDIR)/tools/openfst/lib -lfst -lkaldi-base -lkaldi-online2 -lkaldi-chain -lkaldi-kws -lkaldi-sgmm2 -lkaldi-cudamatrix -lkaldi-lat -lkaldi-decoder -lkaldi-lm -lkaldi-thread -lkaldi-feat -lkaldi-matrix -lkaldi-transform -lkaldi-fstext -lkaldi-nnet2 -lkaldi-tree -lkaldi-gmm -lkaldi-nnet3 -lkaldi-util -lkaldi-hmm -lkaldi-nnet -DKALDI_DOUBLEPRECISION=0 -DHAVE_OPENFST_GE_10400 -DHAVE_ATLAS -DHAVE_CXXABI_H


all:	fl1_morph fl2_tagger fl3_chunker fl4_dependences ident coreferences sample kbest_tagger alts analyzer summary embeddings corrector asr

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

embeddings: embeddings.cc
	g++ -std=gnu++0x  -o embeddings embeddings.cc $(PATHS) $(LIBS)

corrector: corrector.cc
	g++ -std=gnu++0x  -o corrector corrector.cc $(PATHS) $(LIBS)

asr: asr.cc
	@if (test $(KALDIDIR) == none); then \
	  echo "KALDIDIR not defined. Not compiling 'asr.cc' example."; \
	else \
	  echo g++ -o asr asr.cc $(PATHS) -std=gnu++0x -lrt -lasr $(PATHS) $(LIBS) $(KALDI); \
	  g++ -o asr asr.cc $(PATHS) -std=gnu++0x -lrt -lasr $(PATHS) $(LIBS) $(KALDI); \
	fi

clean:
	rm -f fl1_morph fl2_tagger fl3_chunker fl4_dependences ident coreferences sample kbest_tagger alts analyzer summary embeddings corrector asr

