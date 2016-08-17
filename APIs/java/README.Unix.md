## Build Freeling Java API under Linux/MacOS

### HOW TO BUILD THE API

  1. [Install FreeLing](https://talp-upc.gitbooks.io/freeling-user-manual/content/installation.html)

    - OSX: `brew install freeling`
    - Linux: Able to install via packages.

  2. [Install Java SDK and JNI headers](http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html)

  3. Set/Edit Your ENV variables, which in the Makefile:

  Variable       | Description                                                       | Default                    |
  :------------- | :---------------------------------------------------------------- | :------------------------- |
  `FREELINGDIR`  | directory prefix where you installed freeling                     | /usr/local                 |
  `FREELINGOUT`  | directory prefix where your lib will be placed                    | same directory             |
  `SWIGDIR`      | directory where swig share files are installed in your system     | usr/local/share/swig2.0    |
  `JAVADIR`      | directory where java is installed                                 | /usr/lib/jvm/java-7-oracle |

  **OSX ONLY**

  Variable         | Description                                   | Default                |
  :--------------- | :-------------------------------------------- | :--------------------- |
  `ICU4CDIR`       | directory where icu4c is installed            | /usr/local/opt/icu4c   |
  `SYSTEMLIBS`     | directory where your additional libs is       | /opt/local/lib         |
  `SYSTEMHEADERS`  | directory where your additional headers is    | /opt/local/include     |

  4. Run `make` to build the java API.
     > On `MacOS`, you need to use Makefile.MacOSX: `make -f Makefile.MacOSX`


### HOW TO USE THE API FROM A JAVA PROGRAM

  1. Make sure that the directory contanining `libfreeling.so` `($FREELINGDIR/lib)` is in your `LD_LIBRARY_PATH`

  2. Make sure that the directory contanining `libfreeling_javaAPI.so` (created by `make` above) is in your `LD_LIBRARY_PATH`.

  3. Make sure that the package `freeling.jar` created by make is in your CLASSPATH

  4. Compile and execute the sample programs (or your own java program):

         $ javac Analyzer.java
         $ java Analyzer

         $ javac SemGraph.java
         $ java SemGraph

 See FreeLing documentation and sample programs in src/main to
 understand what this sample program is doing.
