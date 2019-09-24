
# IMPORTANT: If you have FreeLing installed in a path other
# than /usr/local, you can call 'make' overriding variable values,
# E.g.:
#       make FREELINGDIR=/my/freeling/dir

# IMPORTANT:  Depending on how you installed ruby, you may get an
# a compilation error about "ruby/config.h not found". If that
# is the case, you need to add an extra "-I" option with the
# proper path to the "g++" line below.


# this variable may be overriden in the command
# line as shown above
FREELINGDIR = /usr/local

## automatically detects the correct paths for Ruby
RUBY_PATHS = -I$(shell ruby -rrbconfig -e 'puts RbConfig::CONFIG[%q{rubyhdrdir}]') -I$(shell ruby -rrbconfig -e 'puts RbConfig::CONFIG[%q{rubyarchhdrdir}]')

freeling.so: freeling_rubyAPI.cxx
	g++ -shared -o freeling.so freeling_rubyAPI.cxx -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib $(RUBY_PATHS) -fPIC -std=gnu++0x

freeling_rubyAPI.cxx: freeling_rubyAPI.i
	swig -ruby -c++ -o freeling_rubyAPI.cxx freeling_rubyAPI.i

clean:
	rm -f freeling_rubyAPI.cxx freeling.so
