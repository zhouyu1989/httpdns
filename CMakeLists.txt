cmake_minimum_required(VERSION 3.0)
project(httpdns)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I..   -Iinclude -std=c++11 -O2 -DADB_HOST=0 -Wall -Wno-unused-parameter -D_XOPEN_SOURCE -D_GNU_SOURCE")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I..   -Iinclude  -O2 -DADB_HOST=0 -Wall -Wno-unused-parameter -D_XOPEN_SOURCE -D_GNU_SOURCE")

include_directories(include)
aux_source_directory(src DIR_SRCS)
add_library(httpdns SHARED ${DIR_SRCS})
target_link_libraries(httpdns dl pthread cjson curl m)

install(TARGETS httpdns
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	PUBLIC_HEADER DESTINATION include
)

add_subdirectory(sample)
