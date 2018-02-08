

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

#if (NOT DEFINED FREELING_PREFIX)
#  message("")
#  message(FATAL_ERROR "FREELING_PREFIX not defined. Please specify FreeLing installation path with -DFREELING_PREFIX=/path/to/freeling")
#else()
#  if(WIN32)
#    set(FREELING_LIBRARY ${FREELING_PREFIX}/bin/freeling.dll)
#  else()       
#    set(FREELING_LIBRARY ${FREELING_PREFIX}/lib/libfreeling.so)
#  endif()
#endif()

#find_package(Boost COMPONENTS system REQUIRED)
#include_directories(${Boost_INCLUDE_DIRS})

#if (WIN32)
  # Icu component names differ in windows
#  find_package(ICU COMPONENTS dt in uc REQUIRED)
#else()
#  find_package(ICU COMPONENTS data i18n uc REQUIRED)
  #find_package(Threads REQUIRED)
#endif()
#include_directories(${ICU_INCLUDE_DIRS})

#find_package(ZLIB REQUIRED)
#include_directories(${ZLIB_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR}/src/include)

if(PYTHON_API)
  add_subdirectory(python)
endif()

if(JAVA_API)
  add_subdirectory(java)
endif()
