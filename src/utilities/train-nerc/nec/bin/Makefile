
## FreeLing installation dir
FLDIR=/home/usuaris/tools
BOOSTDIR=/usr

all: lexicon train-adaboost test-adaboost test-svm 

test-adaboost : test-adaboost.cc
	g++ -o test-adaboost test-adaboost.cc -lfreeling -I$(FLDIR)/include/ -L$(FLDIR)/lib -I$(BOOSTDIR)/include/ -L$(BOOSTDIR)/lib -std=c++11

train-adaboost : train-adaboost.cc
	g++ -o train-adaboost train-adaboost.cc -lfreeling -I$(FLDIR)/include/ -L$(FLDIR)/lib -I$(BOOSTDIR)/include/ -L$(BOOSTDIR)/lib -std=c++11

test-svm : test-svm.cc
	g++ -o test-svm test-svm.cc -lfreeling -I$(FLDIR)/include/ -L$(FLDIR)/lib -I$(BOOSTDIR)/include/ -L$(BOOSTDIR)/lib -std=c++11

lexicon : lexicon.cc
	g++ -o lexicon lexicon.cc -lfreeling -I$(FLDIR)/include/ -L$(FLDIR)/lib -I$(BOOSTDIR)/include/ -L$(BOOSTDIR)/lib -std=c++11

clean:
	rm -rf *.o test-adaboost train-adaboost test-svm lexicon
