
# Change these paths to match your installation
# Alternatively, you can call 'make' overriding variable values, E.g.:
#     make FREELINGDIR=/my/freeling/dir SWIGDIR=/my/swig/dir JAVADIR=/my/java/dir

# freeling *installation* directory (not source!)
FREELINGDIR ?= /usr/local
# Output directory for compiled library & JNI
FREELINGOUT ?= .
# swig and java location in your system
SWIGDIR ?= /usr/share/swig3.0
JAVADIR ?= /usr/lib/jvm/default-java

# you may change the package name if you want
PACKAGE = edu/upc/freeling
JPACKAGE = edu.upc.freeling

# Change your compiler, if necessary.
GCC ?= g++

all: freeling_javaAPI.cxx libJfreeling.so

.PHONY: all

# Compile the C++ API
libJfreeling.so: freeling_javaAPI.cxx
	$(GCC) -shared -o $(FREELINGOUT)/libJfreeling.so freeling_javaAPI.cxx -lfreeling -L$(FREELINGDIR)/lib -lboost_system -I$(FREELINGDIR)/include -I$(FREELINGDIR)/include/treeler -I$(FREELINGDIR)/include/crfsuite -I$(JAVADIR)/include -I$(JAVADIR)/include/linux -fPIC -std=c++0x
	$(JAVADIR)/bin/javac $(PACKAGE)/*.java
	$(JAVADIR)/bin/jar -cf $(FREELINGOUT)/Jfreeling.jar $(PACKAGE)

# Build the Java JNI wrapper classes.
freeling_javaAPI.cxx: freeling_javaAPI.i ../common/freeling.i
	rm -rf $(PACKAGE)
	mkdir -p $(PACKAGE)
	mkdir -p $(FREELINGOUT)
	swig -java -c++ -package $(JPACKAGE) -outdir $(PACKAGE) -o freeling_javaAPI.cxx -I$(SWIGDIR)/java -I$(SWIGDIR)/std -I$(SWIGDIR) freeling_javaAPI.i

clean:
	rm -rf $(FREELINGOUT)/*.jar $(FREELINGOUT)/*.so $(FREELINGOUT)/*.dylib *.cxx *.class edu
