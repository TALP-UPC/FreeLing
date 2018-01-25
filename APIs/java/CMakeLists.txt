
find_package(Java REQUIRED COMPONENTS Runtime Development)
find_package(JNI REQUIRED)
include(UseJava)
INCLUDE_DIRECTORIES(${JNI_INCLUDE_DIRS})

find_library(FREELING freeling
             PATHS ${FREELING_PREFIX}/lib)

INCLUDE_DIRECTORIES(${FREELING_PREFIX}/include)

SET_PROPERTY(SOURCE freeling_javaAPI.i PROPERTY CPLUSPLUS ON)
set (CMAKE_CXX_STANDARD 11)

set(CMAKE_SWIG_FLAGS -package edu.upc.freeling)
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/edu/upc/freeling)
SWIG_ADD_LIBRARY(Jfreeling TYPE SHARED LANGUAGE java SOURCES freeling_javaAPI.i)
SWIG_LINK_LIBRARIES(Jfreeling ${FREELING_PREFIX}/lib/libfreeling.so)

ADD_CUSTOM_COMMAND(TARGET Jfreeling POST_BUILD
    COMMAND "${Java_JAVAC_EXECUTABLE}" ${CMAKE_CURRENT_BINARY_DIR}/edu/upc/freeling/*.java
    COMMAND "${Java_JAR_EXECUTABLE}" -cf freeling.jar edu/upc/freeling/*.class
 )

# This install the APIs in /usr/local/APIs... Need to think of a better place
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/libJfreeling.so ${CMAKE_CURRENT_BINARY_DIR}/freeling.jar
#        DESTINATION APIs/java)

