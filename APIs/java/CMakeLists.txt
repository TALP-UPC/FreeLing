
find_package(Java REQUIRED COMPONENTS Runtime Development)
find_package(JNI REQUIRED)
include(UseJava)
INCLUDE_DIRECTORIES(${JNI_INCLUDE_DIRS})

SET_PROPERTY(SOURCE freeling_javaAPI.i PROPERTY CPLUSPLUS ON)

set(CMAKE_SWIG_FLAGS -package edu.upc.Jfreeling)
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/edu/upc/Jfreeling)
SWIG_ADD_LIBRARY(Jfreeling TYPE SHARED LANGUAGE java SOURCES freeling_javaAPI.i)

SWIG_LINK_LIBRARIES(Jfreeling freeling)

ADD_CUSTOM_COMMAND(TARGET Jfreeling POST_BUILD
    COMMAND "${Java_JAVAC_EXECUTABLE}" ${CMAKE_CURRENT_BINARY_DIR}/edu/upc/Jfreeling/*.java
    COMMAND "${Java_JAR_EXECUTABLE}" -cf Jfreeling.jar edu/upc/Jfreeling/*.class
 )

install(TARGETS Jfreeling
        DESTINATION share/freeling/APIs/java)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/Jfreeling.jar ${CMAKE_CURRENT_SOURCE_DIR}/Sample.java
        DESTINATION share/freeling/APIs/java)
