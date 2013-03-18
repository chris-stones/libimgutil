cmake_minimum_required(VERSION 2.8)

project(imgutil)

add_definitions(-DWITH_IMGU_ROTATE)
add_definitions(-DWITH_IMGU_PAD)

include_directories ("${PROJECT_SOURCE_DIR}")

FILE(GLOB c_source_files *.c)

add_library(imgutil SHARED ${c_source_files} )

install (TARGETS imgutil DESTINATION lib)
install (FILES libimgutil.h DESTINATION include)

target_link_libraries (imgutil img txc_dxtn)
