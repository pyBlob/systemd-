# Copyright 2017-2017 Karl Kraus. See LICENSE for legal info.

cmake_minimum_required(VERSION 2.8.0)

find_library(SD_LIBRARY systemd REQUIRED)

include_directories(src PUBLIC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++14 -g")
add_library(systemd++ SHARED
	src/libsystemd++/sd-event/sd-event.cpp
)
target_link_libraries(systemd++
	${SD_LIBRARY}
)

install(DIRECTORY src/systemd++ DESTINATION include)
install(TARGETS systemd++ LIBRARY DESTINATION lib)

