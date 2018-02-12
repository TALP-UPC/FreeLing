
find_package( PythonInterp 2.7 REQUIRED )
find_package( PythonLibs 2.7 REQUIRED )

INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})

SET_PROPERTY(SOURCE freeling_pythonAPI.i PROPERTY CPLUSPLUS ON)

SWIG_ADD_LIBRARY(pyfreeling TYPE SHARED LANGUAGE python SOURCES freeling_pythonAPI.i)

if(WIN32)
  SWIG_LINK_LIBRARIES(pyfreeling freeling ${PYTHON_LIBRARIES} ${Boost_LIBRARIES})
else()
  SWIG_LINK_LIBRARIES(pyfreeling freeling)
endif()

install(TARGETS _pyfreeling DESTINATION share/freeling/APIs/python2)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/pyfreeling.py ${CMAKE_CURRENT_SOURCE_DIR}/sample.py
        DESTINATION share/freeling/APIs/python2)



