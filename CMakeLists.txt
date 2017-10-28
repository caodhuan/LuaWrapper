cmake_minimum_required(VERSION 2.8)


project(LuaWrapper)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB libluafilesrc )

foreach(files lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c ltm.c lundump.c lvm.c lzio.c lauxlib.c lbaselib.c lbitlib.c lcorolib.c ldblib.c liolib.c lmathlib.c loslib.c lstrlib.c ltablib.c lutf8lib.c loadlib.c linit.c)
	list(APPEND libluafilesrc "${CMAKE_CURRENT_SOURCE_DIR}/lua-5.3.4/src/${files}")
endforeach()


file(GLOB wrappersrc
	${CMAKE_CURRENT_SOURCE_DIR}/wrapper/*.*
	)
source_group(wrapper FILES ${wrappersrc} )

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/lua-5.3.4/src/
	${CMAKE_CURRENT_SOURCE_DIR}/wrapper/
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11")

add_library(lua ${libluafilesrc} ${wrappersrc})

add_executable(Tests tests/main.cpp)

target_link_libraries(Tests lua)