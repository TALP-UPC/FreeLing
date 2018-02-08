
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/src/include)

if(PYTHON_API)
  add_subdirectory(python)
endif()

if(JAVA_API)
  add_subdirectory(java)
endif()
