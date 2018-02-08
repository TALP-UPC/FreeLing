
FIND_PACKAGE(PythonLibs 3)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
#INCLUDE_DIRECTORIES(${FREELING_PREFIX}/include)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ICU_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})

link_directories(${Boost_LIBRARY_DIRS})
link_directories(${ICU_LIBRARY_DIRS})


set (CMAKE_CXX_STANDARD 11)
add_definitions( -DBOOST_ALL_NO_LIB  -DBOOST_ALL_DYN_LINK )
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

if (WIN32)
  # Swig-generated code throws linking errors in Release mode
  SET(CMAKE_BUILD_TYPE Debug)
  # compiler definitions for windows
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -DNOMINMAX -DNDEBUG=1 -DWIN32=1 -D_CONSOLE=1 -DSWIG_PYTHON_INTERPRETER_NO_DEBUG)
endif()

SET_PROPERTY(SOURCE freeling_pythonAPI.i PROPERTY CPLUSPLUS ON)

SET(CMAKE_SWIG_FLAGS "-py3")
SWIG_ADD_LIBRARY(pyfreeling TYPE SHARED LANGUAGE python SOURCES freeling_pythonAPI.i)

if(WIN32)
  SWIG_LINK_LIBRARIES(pyfreeling freeling ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
else()
  SWIG_LINK_LIBRARIES(pyfreeling freeling)
endif()


#install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/freeling.py ${CMAKE_CURRENT_BINARY_DIR}/freeling.lib
#        DESTINATION APIs/python)
