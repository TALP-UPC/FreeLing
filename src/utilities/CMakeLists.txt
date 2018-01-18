
include_directories(${Boost_INCLUDE_DIR})
include_directories(${ICU_INCLUDE_DIRS})

if (WIN32)
  # compiler defintions for windows
  add_definitions(-D_CRT_SECURE_NO_WARNINGS=1 -D_CRT_NONSTDC_NO_DEPRECATE=1 -DNOMINMAX -DNDEBUG=1 -DWIN32=1 -D_CONSOLE=1)
  # avoid creating "Debug|Release" directory
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/src/utilities)
  SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/src/utilities)
endif()

# build-dict
add_executable(build-dict installation/build-dict.cc)
target_link_libraries(build-dict ${Boost_LIBRARIES})

# fusion-mw
add_executable(fusion-mw installation/fusion-mw.cc)
target_link_libraries(fusion-mw ${Boost_LIBRARIES})

# dicc2phon
add_executable(dicc2phon installation/dicc2phon.cc)
target_link_libraries(dicc2phon freeling)

# gz-cat
add_executable(gz-cat installation/gz-cat.cc)
target_link_libraries(gz-cat freeling ${Boost_LIBRARIES})

# convert_model
add_executable(convert_model embeddings/convert_model.cc)
target_link_libraries(convert_model freeling)

install(TARGETS convert_model
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)

