
cmake_minimum_required(VERSION 3.7)

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# POSAR OPCIONS QUE HO FACIN NOMES SI CAL

add_subdirectory(python)
#add_subdirectory(java)
#add_subdirectory(perl)

