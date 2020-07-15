
find_package(PerlLibs 5 REQUIRED )

INCLUDE_DIRECTORIES(${PERL_INCLUDE_PATH})

SET_PROPERTY(SOURCE freeling_perlAPI.i PROPERTY CPLUSPLUS ON)

SWIG_ADD_LIBRARY(plfreeling TYPE SHARED LANGUAGE perl SOURCES freeling_perlAPI.i)

if(WIN32)
  SWIG_LINK_LIBRARIES(plfreeling freeling ${PERL_LIBRARY} ${Boost_LIBRARIES})
else()
  SWIG_LINK_LIBRARIES(plfreeling freeling)
endif()

install(TARGETS plfreeling DESTINATION share/freeling/APIs/perl)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/plfreeling.pm ${CMAKE_CURRENT_SOURCE_DIR}/sample.perl
        DESTINATION share/freeling/APIs/perl)



