
# You may need to change these paths to match your installation
# Alternatively, you can call 'make' overriding variable values, E.g.:
#     make FREELINGDIR=/my/freeling/dir PERLDIR=/my/perl/dir

FREELINGDIR = /usr/local/
PERLDIR = /usr/lib/perl/5.18

freeling.so: freeling_perlAPI.cxx
	g++ -shared -o freeling.so freeling_perlAPI.cxx -lfreeling -I $(FREELINGDIR)/include -L $(FREELINGDIR)/lib -I $(PERLDIR)/CORE -fPIC -std=c++0x

freeling_perlAPI.cxx: freeling_perlAPI.i ../common/freeling.i
	swig -perl5 -c++ -o freeling_perlAPI.cxx freeling_perlAPI.i

clean:
	rm -f freeling_perlAPI.cxx freeling.so freeling.pm
