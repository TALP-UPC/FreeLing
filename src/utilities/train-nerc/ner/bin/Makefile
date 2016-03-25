
## change this if FreeLing is installed somewhere else
LIBDIR=/home/usuaris/tools/
BOOSTDIR=/usr

all: lexicon train-adaboost train-viterbi test-adaboost test-svm

test-adaboost : test-adaboost.cc
	g++ -o test-adaboost test-adaboost.cc -lfreeling -I$(LIBDIR)/include/ -L$(LIBDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib

test-svm : test-svm.cc
	g++ -o test-svm test-svm.cc -lfreeling -I$(LIBDIR)/include/ -L$(LIBDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib

train-adaboost : train-adaboost.cc
	g++ -o train-adaboost train-adaboost.cc -lfreeling -I$(LIBDIR)/include/ -L$(LIBDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib

train-viterbi : train-viterbi.cc
	g++ -o train-viterbi train-viterbi.cc -lfreeling -I$(LIBDIR)/include/ -L$(LIBDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib

lexicon : lexicon.cc
	g++ -o lexicon lexicon.cc -lfreeling -I$(LIBDIR)/include/ -L$(LIBDIR)/lib -I$(BOOSTDIR)/include -L$(BOOSTDIR)/lib

clean:
	rm -f lexicon train-adaboost train-viterbi test-adaboost test-svm *.o

