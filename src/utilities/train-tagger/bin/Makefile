
# Adjust this path to your FreeLing installation
FREELINGDIR=/usr/local

all: update-probs change-corpus-dict add-corpus-dict

update-probs: update-probs.cc
	g++ -o update-probs update-probs.cc -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -std=gnu++0x -lboost_system

change-corpus-dict: change-corpus-dict.cc
	g++ -o change-corpus-dict change-corpus-dict.cc -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -std=gnu++0x -lboost_system

add-corpus-dict: add-corpus-dict.cc
	g++ -o add-corpus-dict add-corpus-dict.cc -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -std=gnu++0x -lboost_system -lboost_filesystem

clean: 
	rm -f update-probs change-corpus-dict add-corpus-dict
