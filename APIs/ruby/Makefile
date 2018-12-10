
# IMPORTANT: If you have FreeLing installed in a path other
# than /usr/local, and/or your ruby version is different
# than 2.5.0, you can call 'make' overriding variable values,
# E.g.:
#       make FREELINGDIR=/my/freeling/dir RUBYVER=ruby-X.X.X

# IMPORTANT:  Depending on how you installed ruby, you may get an
# a compilation error about "ruby/config.h not found". If that
# is the case, you need to add an extra "-I" option with the
# proper path to the "g++" line below.


# these two variables may be overriden in the command
# line as shown above
FREELINGDIR = /usr/local
RUBYVER = ruby-2.5.0

## change this if you manually installed ruby somewhere else
RUBYDIR = /usr/include/$(RUBYVER)

freeling.so: freeling_rubyAPI.cxx
	g++ -shared -o freeling.so freeling_rubyAPI.cxx -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -I$(RUBYDIR) -I/usr/include/x86_64-linux-gnu/$(RUBYVER) -I$(RUBYDIR)/x86_64-linux -fPIC -std=gnu++0x

freeling_rubyAPI.cxx: freeling_rubyAPI.i
	swig -ruby -c++ -o freeling_rubyAPI.cxx freeling_rubyAPI.i
	cat freeling_rubyAPI.cxx \
	| sed 's/make_set_nonconst_iterator/make_nonconst_iterator/' \
	| sed 's/value_type& dst = \*base::current;/value_type dst = *base::current;/' \
	> tmp; \
	mv tmp freeling_rubyAPI.cxx

clean:
	rm -f freeling_rubyAPI.cxx freeling.so
