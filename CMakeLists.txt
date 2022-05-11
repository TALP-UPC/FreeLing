cmake_minimum_required(VERSION 3.12)

# disallow in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
   message("")
   message(" You are attempting to build in your source Directory.")
   message(" You should run cmake from a build directory.")
   message("")
   message(FATAL_ERROR "Build aborted.")
endif()

project(FreeLing)
set(VERSION "4.3_devel") 
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
option(PYTHON3_API "Build Python 3 API" OFF)

# Check for dependencies -- ZLIB
find_package(ZLIB REQUIRED)

# Check for dependencies -- Boost
#set(Boost_DEBUG ON)
if(BUILD_TESTS)
  find_package(Boost COMPONENTS regex filesystem thread program_options unit_test_framework REQUIRED)
else()
  find_package(Boost COMPONENTS regex filesystem thread program_options REQUIRED)
endif()

# Check for dependencies -- ICU
if (WIN32)
  # Icu component names differ in windows
  find_package(ICU COMPONENTS dt in uc REQUIRED)
else()
  #  find_package(ICU COMPONENTS data i18n uc REQUIRED)
  find_package(ICU REQUIRED COMPONENTS data i18n uc)
  find_package(Threads REQUIRED)
endif()

# All sources are under this directory
add_subdirectory(src)

# if any APIs were requested, build them
if(PYTHON3_API OR JAVA_API OR PERL_API)
  add_subdirectory(APIs)
endif()

#### Data installation hooks

# see if a list of languages to install was specified
if (NOT DEFINED LANGUAGES)
   # find out which language data are available
   file(GLOB LANGUAGES RELATIVE ${CMAKE_SOURCE_DIR}/data CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/data/??)
endif()
MESSAGE(STATUS "Configuring languages: [${LANGUAGES}]")

# see if a list of languages to install was specified
if (NOT DEFINED VARIANTS)
   # just in case, will be ignored if ca/es are not to be installed
   SET(VARIANTS "es/es-old;es/es-ar;es/es-cl;ca/balear;ca/valencia")      
endif()
MESSAGE(STATUS "Configuring variants: [${VARIANTS}]")

foreach (lg ${LANGUAGES})
   install(DIRECTORY data/${lg}
           DESTINATION share/freeling
           PATTERN "dictionary" EXCLUDE
	   PATTERN "parameters.*.gz.*" EXCLUDE
	   PATTERN "model*.gz.*" EXCLUDE
	   PATTERN "*embeddings*.gz.*" EXCLUDE
	   PATTERN "model*.crf.*" EXCLUDE
	   PATTERN "Makefile*" EXCLUDE)
endforeach()
install(DIRECTORY data/common
        DESTINATION share/freeling)
install(DIRECTORY data/config
        DESTINATION share/freeling)

if (WIN32)
  # set library path for installation programs
  install(CODE "set(ENV{PATH} \"${Boost_LIBRARY_DIRS};${ICU_INCLUDE_DIRS}/../bin64;${ZLIB_INCLUDE_DIRS}/../bin;${CMAKE_BINARY_DIR}/src/libfreeling;${CMAKE_BINARY_DIR}/src/libfoma;${CMAKE_BINARY_DIR}/src/libtreeler;${CMAKE_BINARY_DIR}/src/crfsuite;${CMAKE_BINARY_DIR}/src/libdynet/dynet;\")")
endif()

foreach (lg ${LANGUAGES})
  
  # build dictionary for each language
  if (EXISTS ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/header)
    install(CODE "message(STATUS \"Creating ${lg} dictionary...\")" )
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/build-dict ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/header ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/entries  OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/dicc.src RESULT_VARIABLE _res) " )
    # make sure installation helpers work.  If this one works, the others should too.
    install(CODE "If (NOT \${_res} EQUAL \"0\")
                     message(FATAL_ERROR \"Error '\${_res}' while executing dictionary indexation program. Make sure compilation ended without errors, and that boost libraries can be found in the path. \")
                  endif() ")
  endif()
  
  foreach (variant ${VARIANTS})
    # for variants matching this language, build specific dictionary and multiwords file
    if (${variant} MATCHES "^${lg}/")
      install(CODE "message(STATUS \"-- Creating ${variant} dictionary...\")" )
      install(CODE "file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant})")      
      install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/build-dict ${CMAKE_SOURCE_DIR}/data/${variant}/dictionary/header ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/entries ${CMAKE_SOURCE_DIR}/data/${variant}/dictionary/entries ${CMAKE_SOURCE_DIR}/data/${variant}/remove OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant}/dicc.src RESULT_VARIABLE _res)" )
      install(CODE "If (NOT \${_res} EQUAL \"0\")
                        message(FATAL_ERROR \"dictionary fusion failed with error: \${_res}\")
                    endif() ")
      if (EXISTS ${CMAKE_SOURCE_DIR}/data/${variant}/locucions.dat)
        install(CODE "message(STATUS \"-- Merging ${variant} multiwords...\")" )
	install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/fusion-mw ${CMAKE_SOURCE_DIR}/data/${variant}/locucions.dat ${CMAKE_SOURCE_DIR}/data/${lg}/locucions.dat OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant}/locucions.dat RESULT_VARIABLE _res)" )
        install(CODE "If (NOT \${_res} EQUAL \"0\")
                        message(FATAL_ERROR \"multiword fusion failed with error: \${_res}\")
                      endif() ")
      endif()
    endif()
  endforeach()

  # build phonetic dictionary if needed
  if (EXISTS ${CMAKE_SOURCE_DIR}/data/${lg}/alternatives-phon.dat)
    install(CODE "message(STATUS \"-- Creating ${lg} phonetic dictionary...\")" )
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/dicc2phon ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/dicc.src ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/alternatives-phon.dat RESULT_VARIABLE _res)" )
    install(CODE "If (NOT \${_res} EQUAL \"0\")
                      message(FATAL_ERROR \"creating phonetic dictionary failed with error: \${_res}\")
                  endif() ")
  endif()

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
    install(CODE "message(STATUS \"-- Rebuilding data file ${gzfile}\")" )
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/gz-cat ${model} ${gzfile} RESULT_VARIABLE _res)")
    install(CODE "If (NOT \${_res} EQUAL \"0\")
                    message(FATAL_ERROR \"zcat failed with error: \${_res}\")
                  endif() ")
  endforeach()    
  
endforeach()


#Setup CMake to run tests
enable_testing()
add_test(NAME fl_test
         COMMAND fl_test ${CMAKE_INSTALL_PREFIX})
