
# You may need to change these paths to match your installation
# Alternatively, you can call 'make' overriding variable values, E.g.:
#     make FREELINGDIR=/my/freeling/dir RUBYVER=ruby-X.X.X

# set FREELINGDIR to folder containing FreeLing installation
FREELINGDIR = /usr/local
# set RUBYVER to folder containing ruby.h development headers
RUBYVER = ruby-2.5.0
RUBYDIR = /usr/include/$(RUBYVER)

freeling.so: freeling_rubyAPI.cxx
	g++ -shared -o freeling.so freeling_rubyAPI.cxx -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -I$(RUBYDIR) -I/usr/include/x86_64-linux-gnu/$(RUBYVER) -fPIC -std=gnu++0x

freeling_rubyAPI.cxx: freeling_rubyAPI.i
	swig -ruby -c++ -o freeling_rubyAPI.cxx freeling_rubyAPI.i
	cat freeling_rubyAPI.cxx \
	| sed 's/make_set_nonconst_iterator/make_nonconst_iterator/' \
	| sed 's/value_type& dst = \*base::current;/value_type dst = *base::current;/' \
	> tmp; \
	mv tmp freeling_rubyAPI.cxx

clean:
	rm -f freeling_rubyAPI.cxx freeling.so
