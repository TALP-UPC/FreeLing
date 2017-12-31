
find_package(Boost COMPONENTS regex system filesystem thread REQUIRED)
include_directories(${Boost_INCLUDE_DIR})


# build-dict
add_executable(build-dict installation/build-dict.cc)
target_link_libraries(build-dict ${Boost_LIBRARIES})

# fusion-mw
add_executable(fusion-mw installation/fusion-mw.cc)
target_link_libraries(fusion-mw ${Boost_LIBRARIES})

# dicc2phon
add_executable(dicc2phon installation/dicc2phon.cc)
target_link_libraries(dicc2phon freeling ${Boost_LIBRARIES})

# gz-cat
add_executable(gz-cat installation/gz-cat.cc)
target_link_libraries(gz-cat freeling ${Boost_LIBRARIES})


# convert_model <- not "corrector" related, to be moved to somewhere else
add_executable(convert_model corrector/convert_model.cc)
target_link_libraries(convert_model freeling ${Boost_LIBRARIES})


