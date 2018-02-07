
SET(CMAKE_BUILD_TYPE Debug)

FIND_PACKAGE(PythonLibs 3)
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${FREELING_PREFIX}/include)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${ICU_INCLUDE_DIRS})
include_directories(${ZLIB_INCLUDE_DIRS})

link_directories(${FREELING_PREFIX}/lib/static)
link_directories(${FREELING_PREFIX}/lib)
link_directories(${FREELING_PREFIX}/bin)

link_directories(${Boost_LIBRARY_DIRS})
link_directories(${ICU_LIBRARY_DIRS})


set (CMAKE_CXX_STANDARD 11)
add_definitions( -DBOOST_ALL_NO_LIB  -DBOOST_ALL_DYN_LINK )
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

if (WIN32)
  # compiler defintions for windows
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -DNOMINMAX -DNDEBUG=1 -DWIN32=1 -D_CONSOLE=1 -DSWIG_PYTHON_INTERPRETER_NO_DEBUG)
endif()

SET_PROPERTY(SOURCE freeling_pythonAPI.i PROPERTY CPLUSPLUS ON)



#SET(CMAKE_SWIG_FLAGS "-py3")
#SWIG_ADD_LIBRARY(pyfreeling TYPE SHARED LANGUAGE python SOURCES freeling_pythonAPI.i)
#if(WIN32)
#  file(TO_CMAKE_PATH ${FREELING_PREFIX} FREELING_PREFIX)
#  SWIG_LINK_LIBRARIES(pyfreeling ${PYTHON_LIBRARIES} ${FREELING_PREFIX}/lib/static/freeling.lib ${Boost_LIBRARIES})
#  #SWIG_LINK_LIBRARIES(pyfreeling ${PYTHON_LIBRARIES} ${FREELING_PREFIX}/bin/freeling.dll ${Boost_LIBRARIES})
#else()
#  SWIG_LINK_LIBRARIES(pyfreeling ${FREELING_PREFIX}/lib/libfreeling.so)
#endif()


### PROVAR DE GENERAR EL cxx AMB EL SWIG, ara que sabem que es lo del "debug".  Tambe hauria de funcionar linkar amb "freeling" a seques.
add_custom_command(OUTPUT freeling_pythonAPI.cxx
                   COMMAND ${SWIG_EXECUTABLE} -python -py3 -c++ -o freeling_pythonAPI.cxx ${CMAKE_CURRENT_SOURCE_DIR}/freeling_pythonAPI.i)


if(WIN32)
  add_library(pyfreeling SHARED freeling_pythonAPI.cxx )
  set_target_properties(pyfreeling PROPERTIES OUTPUT_NAME "_freeling")
  set_target_properties(pyfreeling PROPERTIES PREFIX "")
  target_link_libraries(pyfreeling freeling ${PYTHON_LIBRARIES} ${Boost_SYSTEM_LIBRARIES})
  set_target_properties(pyfreeling PROPERTIES LINKER_LANGUAGE CXX)

else()
  add_library(pyfreeling SHARED freeling_pythonAPI.cxx)
  set_target_properties(pyfreeling PROPERTIES OUTPUT_NAME "_freeling")
  set_target_properties(pyfreeling PROPERTIES PREFIX "")
  target_link_libraries(pyfreeling freeling)
  set_target_properties(pyfreeling PROPERTIES LINKER_LANGUAGE CXX)
endif()


#install(FILES  ${CMAKE_CURRENT_BINARY_DIR}/freeling.py ${CMAKE_CURRENT_BINARY_DIR}/freeling.lib
#        DESTINATION APIs/python)
