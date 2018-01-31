
cmake_minimum_required(VERSION 3.7)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

option(JAVA_API "Build Java API" OFF)
option(PYTHON_API "Build Python API" OFF)
option(PERL_API "Build Perl API" OFF)

if (NOT DEFINED FREELING_PREFIX)
  message("")
  message(FATAL_ERROR "FREELING_PREFIX not defined. Please specify FreeLing installation path with -DFREELING_PREFIX=/path/to/freeling")
endif()

if(PYTHON_API)
  add_subdirectory(python)
endif()

if(JAVA_API)
  add_subdirectory(java)
endif()
