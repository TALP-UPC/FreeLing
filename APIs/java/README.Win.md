## Build Freeling Java API under WINDOWS

### HOW TO BUILD THE API USING MSVC

  1. Install java 

  2. Download and install swig (http://www.swig.org/)

  3. Run swig with freeling_javaAPI.i to generate the API
     SWIG will generate a directory edu/upc/freeling with the Java API 
     for all modules, and a file freeling_javaAPI.cxx with the JNI side 
     of the API.

  4. Compile java code in edu/upc/freeling and build a file `freeling.jar`
     with the results.

  5. Compile `freeling_javaAPI.cxx` into a DLL (using MSVC or any other compiler).
     You'll need to provide paths to freeling and treeler include directories, 
     and to Java headers (jni.h etc)
     
     You will get a freeling_javaAPI.dll library to be loaded from 
     your Java programs


### HOW TO USE THE API FROM A JAVA PROGRAM

  1. Make sure `libfreeling.dll`, `libboost`, and `libicu` are in a path where 
      they will be found.
      > (e.g. set `PATH=%PATH%;C:\my\freeling\installation;C:\my\libicu\installation`)

  2. Make sure `libfreeling_javaAPI.dll` (created by swig) is in a path 
      where it will be found.
      > (e.g. set `PATH=%PATH%;C:\my\freeling\javaAPI\installation`)

  3. Make sure that the package `freeling.jar` created by make is in your `CLASSPATH`

  4. Compile and execute the sample program (or your program): 
  
         C:\my\java\installation\javac.exe Analyzer.java
         C:\my\java\installation\java.exe Analyzer

 See FreeLing documentation and sample programs in src/main to
 understand what this sample program is doing.

