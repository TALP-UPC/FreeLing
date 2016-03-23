
# You may need to change these paths to match your installation
# Alternatively, you can call 'make' overriding variable values, E.g.:
#     make FREELINGDIR=/my/freeling/dir RUBYDIR=/my/ruby/dir

FREELINGDIR = /usr/local
RUBYDIR = /usr/include/ruby-1.9.1

freeling.so: freeling_rubyAPI.cxx
	g++ -shared -o freeling.so freeling_rubyAPI.cxx -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -I$(RUBYDIR) -I$(RUBYDIR)/x86_64-linux -fPIC -std=gnu++0x

freeling_rubyAPI.cxx: freeling_rubyAPI.i
	swig -ruby -c++ -o freeling_rubyAPI.cxx freeling_rubyAPI.i

clean:
	rm -f freeling_rubyAPI.cxx freeling.so
