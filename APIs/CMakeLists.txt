
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/src/include)

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

if(PYTHON2_API AND PYTHON3_API)
  message(FATAL_ERROR "\nCan not build both python2 and python3 APIs. Please select only one.")
endif()

if(PYTHON3_API)
  add_subdirectory(python3)
elseif(PYTHON2_API)
  add_subdirectory(python2)
endif()

if(JAVA_API)
  add_subdirectory(java)
endif()
