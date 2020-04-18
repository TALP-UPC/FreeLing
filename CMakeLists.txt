cmake_minimum_required(VERSION 3.8)

# disallow in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
   message("")
   message(" You are attempting to build in your source Directory.")
   message(" You should run cmake from a build directory.")
   message("")
   message(FATAL_ERROR "Build aborted.")
endif()

project(FreeLing)
set(VERSION "4.1") 
set(PACKAGE_STRING "\"${CMAKE_PROJECT_NAME} ${VERSION}\"")
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib") # Needed to find libraries if not installed in system path

# Add compiler definitions
add_definitions(-DPACKAGE_STRING=${PACKAGE_STRING} -DVERSION=${VERSION})

# Global configuration
set(CMAKE_CXX_STANDARD 11)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# compilation options
option(BUILD_TESTS "Build tests" OFF)
option(TRACES "Enable traces" OFF)
option(WARNINGS "Enable warnings" ON)
option(XPRESSIVE "Xpressive regex" OFF)
option(JAVA_API "Build Java API" OFF)
option(PERL_API "Build Perl API" OFF)
option(PYTHON2_API "Build Python 2.7 API" OFF)
option(PYTHON3_API "Build Python 3 API" OFF)

# Check for dependencies -- ZLIB
find_package(ZLIB REQUIRED)

# Check for dependencies -- Boost
if(BUILD_TESTS)
  find_package(Boost COMPONENTS regex filesystem thread program_options iostreams unit_test_framework REQUIRED)
else()
  find_package(Boost COMPONENTS regex filesystem thread program_options iostreams REQUIRED)
endif()

# Check for dependencies -- ICU
if (WIN32)
  # Icu component names differ in windows
  find_package(ICU COMPONENTS dt in uc REQUIRED)
else()
  find_package(ICU COMPONENTS data i18n uc REQUIRED)
  find_package(Threads REQUIRED)
endif()

# All sources are under this directory
add_subdirectory(src)

# if any APIs were requested, build them
if(PYTHON3_API OR PYTHON2_API OR JAVA_API OR PERL_API)
  add_subdirectory(APIs)
endif()

install(DIRECTORY data/
        DESTINATION share/freeling
        PATTERN "dictionary" EXCLUDE
	      PATTERN "parameters.*.gz.*" EXCLUDE
        PATTERN "model*.gz.*" EXCLUDE
        PATTERN "*embeddings*.gz.*" EXCLUDE
        PATTERN "model*.crf.*" EXCLUDE
        PATTERN "Makefile*" EXCLUDE)

#### Data installation hooks
SET(languages "as;ca;cs;cy;de;en;es;fr;gl;hr;it;nb;pt;ru;sl")
SET(variants "es/es-old;es/es-ar;es/es-cl;ca/balear;ca/valencia")      

if (WIN32)
  # set library path for installation programs
  install(CODE "set(ENV{PATH} \"${Boost_LIBRARY_DIRS};${ICU_INCLUDE_DIRS}/../bin64;${ZLIB_INCLUDE_DIRS}/../bin;${CMAKE_BINARY_DIR}/src/libfreeling;${CMAKE_BINARY_DIR}/src/libfoma;${CMAKE_BINARY_DIR}/src/libtreeler;\")")
endif()

foreach (lg ${languages})
  # build dictionary for each language
  if (EXISTS ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/header)
    install(CODE "message(STATUS \"Creating ${lg} dictionary...\")" )
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/build-dict ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/header ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/entries  OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/dicc.src RESULT_VARIABLE _res) " )
    # make sure installation helpers work.  If this one works, the others should too.
    install(CODE "If (NOT \${_res} EQUAL \"0\")
                     message(FATAL_ERROR \"Error executing dictionary indexation program. Make sure compilation ended without errors, and that boost libraries can be found in the path.\")
                  endif() ")
  endif()
  
  # build phonetic dictionary
  if (EXISTS ${CMAKE_SOURCE_DIR}/data/${lg}/alternatives-phon.dat)
    install(CODE "message(STATUS \"-- Creating ${lg} phonetic dictionary...\")" )
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/dicc2phon ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/dicc.src ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/alternatives-phon.dat)" )
  endif()

  foreach (variant ${variants})
    # for variants matching this language, build specific dictionary and multiwords file
    if (${variant} MATCHES "^${lg}/")
      install(CODE "message(STATUS \"-- Creating ${variant} dictionary...\")" )
      install(CODE "file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant})")      
      install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/build-dict ${CMAKE_SOURCE_DIR}/data/${variant}/dictionary/header ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/entries ${CMAKE_SOURCE_DIR}/data/${variant}/dictionary/entries ${CMAKE_SOURCE_DIR}/data/${variant}/remove OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant}/dicc.src)" )
      if (EXISTS ${CMAKE_SOURCE_DIR}/data/${variant}/locucions.dat)
	install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/fusion-mw ${CMAKE_SOURCE_DIR}/data/${variant}/locucions.dat ${CMAKE_SOURCE_DIR}/data/${lg}/locucions.dat OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant}/locucions.dat)" )
      endif()
    endif()
  endforeach()

  # if some model files are splitted due to GitHub 50M limitation, join them
  file(GLOB aafiles
            ${CMAKE_SOURCE_DIR}/data/${lg}/treeler/srl/parameters.*.gz.aa
            ${CMAKE_SOURCE_DIR}/data/${lg}/treeler/dep/parameters.*.gz.aa
            ${CMAKE_SOURCE_DIR}/data/${lg}/dep_lstm/model*.gz.aa
            ${CMAKE_SOURCE_DIR}/data/${lg}/embeddings/*embeddings*.gz.aa
            ${CMAKE_SOURCE_DIR}/data/${lg}/nerc/nerc/model*.crf.aa
       )

  foreach (model ${aafiles})
    STRING(REGEX REPLACE ".aa\$" "" gzfile "${model}")
    STRING(REGEX REPLACE "${CMAKE_SOURCE_DIR}/data" "${CMAKE_INSTALL_PREFIX}/share/freeling" gzfile "${gzfile}")
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/gz-cat ${model} ${gzfile})")
  endforeach()    
  
endforeach()

