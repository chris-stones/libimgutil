# HACK!!!
ADD_DEFINITIONS(-fPIC)

FILE(GLOB cxx_source_files *.cpp)

add_library(3rdparty STATIC ${cxx_source_files} )

