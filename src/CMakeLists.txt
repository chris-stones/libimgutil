cmake_minimum_required(VERSION 2.8)

project(imgutil)

set(CMAKE_C_FLAGS_RELEASE "-O4 -march=native")

include_directories ("${PROJECT_SOURCE_DIR}")

add_subdirectory(3rdparty)

FILE(GLOB c_source_files *.c)

add_library(imgutil SHARED ${c_source_files} )

install (TARGETS imgutil DESTINATION lib)
install (FILES libimgutil.h DESTINATION include)

target_link_libraries (imgutil img 3rdparty txc_dxtn csquish pthread)
