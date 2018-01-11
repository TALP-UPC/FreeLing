cmake_minimum_required(VERSION 3.1)

project(FreeLing)
set(PACKAGE_NAME "FreeLing") 
set(VERSION "4.1") 
set(PACKAGE_STRING "\"${PACKAGE_NAME} ${VERSION}\"")
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
option(EMBEDDINGS "Get word Embeddings" OFF)

# All sources are under this directory
add_subdirectory(src)

install(DIRECTORY data/
        DESTINATION share/freeling
        PATTERN "dictionary" EXCLUDE
	PATTERN "parameters.*.gz.*" EXCLUDE
	PATTERN "Makefile*" EXCLUDE)

#### Data installation hooks
      
SET(languages "as;ca;cs;cy;de;en;es;fr;gl;hr;it;nb;pt;ru;sl")
SET(variants "es/es-old;es/es-ar;es/es-cl;ca/balear;ca/valencia")      

foreach (lg ${languages})
  # build dictionary for each language
  if (EXISTS ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/header)
    install(CODE "message(STATUS \"  Creating ${lg} dictionary...\")" )
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/build-dict ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/header ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/entries  OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/dicc.src)" )
  endif()

  # build phonetic dictionary
  if (EXISTS ${CMAKE_SOURCE_DIR}/data/${lg}/alternatives-phon.dat)
    install(CODE "message(STATUS \"  Creating ${lg} phonetic dictionary...\")" )
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/dicc2phon ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/dicc.src ${CMAKE_INSTALL_PREFIX}/share/freeling/${lg}/alternatives-phon.dat)" )
  endif()

  foreach (variant ${variants})
    # for variants matching this language, build specific dictionary and multiwords file
    if (${variant} MATCHES "^${lg}/")
      install(CODE "message(STATUS \"  Creating ${variant} dictionary...\")" )
      install(CODE "file(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant})") 
      install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/build-dict ${CMAKE_SOURCE_DIR}/data/${variant}/dictionary/header ${CMAKE_SOURCE_DIR}/data/${lg}/dictionary/entries ${CMAKE_SOURCE_DIR}/data/${variant}/dictionary/entries ${CMAKE_SOURCE_DIR}/data/${variant}/remove OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant}/dicc.src)" )
      if (EXISTS ${CMAKE_SOURCE_DIR}/data/${variant}/locucions.dat)
	install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/fusion-mw ${CMAKE_SOURCE_DIR}/data/${variant}/locucions.dat ${CMAKE_SOURCE_DIR}/data/${lg}/locucions.dat OUTPUT_FILE ${CMAKE_INSTALL_PREFIX}/share/freeling/${variant}/locucions.dat)" )
      endif()
    endif()
  endforeach()

  # if treeler parameter files are splitted due to GitHub 50M limitation, join them
  file(GLOB aafiles ${CMAKE_SOURCE_DIR}/data/${lg}/dep_treeler/srl/parameters.*.gz.aa ${CMAKE_SOURCE_DIR}/data/${lg}/dep_treeler/dep/parameters.*.gz.aa)
  foreach (model ${aafiles})
    STRING(REGEX REPLACE ".aa\$" "" gzfile "${model}")
    STRING(REGEX REPLACE "${CMAKE_SOURCE_DIR}/data" "${CMAKE_INSTALL_PREFIX}/share/freeling" gzfile "${gzfile}")
    install(CODE "execute_process(COMMAND ${CMAKE_BINARY_DIR}/src/utilities/gz-cat ${model} ${gzfile})")
  endforeach()    
  
endforeach()

if (EMBEDDINGS)
  if (EXISTS ${CMAKE_INSTALL_PREFIX}/share/freeling/en/embeddings)
    install(CODE "message(STATUS \"Word embedding data already installed. To force reinstallation, remove folder ${CMAKE_INSTALL_PREFIX}/share/freeling/en/embeddings and 'make install' again\")" )
  else()
    install(CODE "message(STATUS \"Downloading word embedding data from 'nlp.cs.upc.edu/freeling/extradata'. Please wait...\")" )
    install(CODE "file(DOWNLOAD nlp.cs.upc.edu/freeling/extradata/freeling_embeddings.tar.bz2 ${CMAKE_BINARY_DIR}/freeling_embeddings.tar.bz2 )" ) 
    install(CODE "message(STATUS \"Uncompressing word embeddings data. Please wait...\")" )
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E tar xjf ${CMAKE_BINARY_DIR}/freeling_embeddings.tar.bz2 WORKING_DIRECTORY ${CMAKE_INSTALL_PREFIX}/share/freeling)")
    install(CODE "file(REMOVE ${CMAKE_BINARY_DIR}/freeling_embeddings.tar.bz2 )" ) 
  endif()
endif()
