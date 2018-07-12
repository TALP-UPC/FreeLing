
## Default library directories.
## You can change them calling make with the command
##    make FREELINGDIR=actual/freeling/install/dir  BOOSTDIR=actual/boost/install/dir
FREELINGDIR=/usr/local
BOOSTDIR=/usr/lib

all: lexicon train-adaboost train-viterbi test-adaboost test-svm

test-adaboost : test-adaboost.cc
	g++ -o test-adaboost test-adaboost.cc -lfreeling -I$(FREELINGDIR)/include/ -L$(FREELINGDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib -std=c++11

test-svm : test-svm.cc
	g++ -o test-svm test-svm.cc -lfreeling -I$(FREELINGDIR)/include/ -L$(FREELINGDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib -std=c++11

train-adaboost : train-adaboost.cc
	g++ -o train-adaboost train-adaboost.cc -lfreeling -I$(FREELINGDIR)/include/ -L$(FREELINGDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib -std=c++11

train-viterbi : train-viterbi.cc
	g++ -o train-viterbi train-viterbi.cc -lfreeling -I$(FREELINGDIR)/include/ -L$(FREELINGDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib -std=c++11

lexicon : lexicon.cc
	g++ -o lexicon lexicon.cc -lfreeling -I$(FREELINGDIR)/include/ -L$(FREELINGDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib -std=c++11

clean:
	rm -f lexicon train-adaboost train-viterbi test-adaboost test-svm *.o

