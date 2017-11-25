cmake_minimum_required(VERSION 3.1)

project(FreeLing)
set(PACKAGE_STRING "\"FreeLing\"") # TODO: Get this from proper place
set(VERSION "33") # TODO: Get this from proper place
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib") # Needed to find libraries if not installed in system path

# Add compiler definitions
add_definitions(-DPACKAGE_STRING=${PACKAGE_STRING} -DVERSION=${VERSION})

# Global configuration
set(CMAKE_CXX_STANDARD 11)
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# All sources are under this directory
add_subdirectory(src)
