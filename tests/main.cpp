#include <iostream>

#include <lua.hpp>
using namespace std;

int main() {
	lua_State* state = luaL_newstate();
	
	luaL_openlibs(state);

	luaL_dostring(state, "print(1 + 2)");

	return 0;
}