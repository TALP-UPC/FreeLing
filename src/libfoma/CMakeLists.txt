
file(GLOB_RECURSE sources
    "*.h"
    "*.cpp"
    "*.cc"
)

add_library(foma ${sources})
set_target_properties(foma PROPERTIES LINKER_LANGUAGE CXX)
target_include_directories(foma PUBLIC .)

