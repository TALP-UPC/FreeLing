
# You may need to change these paths to match your installation.
# Alternatively, you can call 'make' overriding variable values, E.g.:
#     make FREELINGDIR=/my/freeling/dir PYTHONDIR=/my/python/dir

FREELINGDIR = /usr/local
PYTHONVER = python3.5
PYTHONDIR = /usr/include/$(PYTHONVER)

_pyfreeling.so: freeling_pythonAPI.cxx
	g++ -shared -o _pyfreeling.so freeling_pythonAPI.cxx -lfreeling -I$(FREELINGDIR)/include -I$(FREELINGDIR)/include/treeler -I$(FREELINGDIR)/include/crfsuite -L$(FREELINGDIR)/lib -I$(PYTHONDIR) -fPIC -std=gnu++0x
#       Mac OSX users:  Comment the line above, and uncomment the line below:
#	g++ -dynamiclib -o _freeling.so freeling_pythonAPI.cxx -lboost_system -l$(PYTHONVER) -lfreeling -I$(FREELINGDIR)/include -L$(FREELINGDIR)/lib -I$(PYTHONDIR) $(CPPFLAGS) $(LDFLAGS) -fPIC  -std=c++0x

freeling_pythonAPI.cxx: freeling_pythonAPI.i ../common/freeling.i ../common/templates.i
	swig -python -py3 -c++ -o freeling_pythonAPI.cxx freeling_pythonAPI.i
#       Python2 users: Remove option "-py3" from the above line

clean:
	rm -rf __pycache__ freeling_pythonAPI.cxx _pyfreeling.so pyfreeling.py*
