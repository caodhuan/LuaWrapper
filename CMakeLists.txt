cmake_minimum_required(VERSION 2.8)


project(LuaWrapper)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_definitions(-DUNICODE -D_UNICODE)

file(GLOB libluafilesrc )

foreach(files lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lbitlib.c lcorolib.c ldblib.c liolib.c lmathlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c loadlib.c linit.c)
	list(APPEND libluafilesrc "${CMAKE_CURRENT_SOURCE_DIR}/3rd/lua-5.3.4/src/${files}")
endforeach()


file(GLOB wrappersrc
	${CMAKE_CURRENT_SOURCE_DIR}/wrapper/*.*
	)
source_group(wrapper FILES ${wrappersrc} )

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/3rd/lua-5.3.4/src/
	${CMAKE_CURRENT_SOURCE_DIR}/wrapper/
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11")

add_library(lua ${libluafilesrc} ${wrappersrc})

# 构造一个头文件，用来标识一些变量
set(COMMONHEADER "${CMAKE_CURRENT_SOURCE_DIR}/tests/common.h")

file(WRITE ${COMMONHEADER} "#pragma once\n")
file(APPEND ${COMMONHEADER} "#define SOURCEPATH \"${CMAKE_CURRENT_SOURCE_DIR}/scripts/\"\n")

add_executable(Tests tests/main.cpp tests/common.h)

target_link_libraries(Tests lua)

# 添加protobuf
file(GLOB pb ${CMAKE_CURRENT_SOURCE_DIR}/3rd/protobuf/cmake/*)
if(pb)
	message(STATUS "found protobuf")
	option(protobuf_BUILD_TESTS OFF)
	add_definitions(-DUSEPROTOBUF)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rd/protobuf/cmake/)

	target_link_libraries(lua libprotobuf)
	
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rd/protobuf/src)

	set_target_properties(libprotobuf PROPERTIES FOLDER libs)
	set_target_properties(libprotobuf-lite PROPERTIES FOLDER libs)
	set_target_properties(libprotoc PROPERTIES FOLDER libs)
	set_target_properties(protoc PROPERTIES FOLDER libs)
	set_target_properties(js_embed PROPERTIES FOLDER libs)

else()
	message("CANT find protobuf")
endif()


if(WIN32)
	set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /Oy- /MTd")
	set(CMAKE_C_FLAGS_DEBUG "/Zi /Od /Oy- /MTd")
else()

endif()