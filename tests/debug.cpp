#include "common.h"
#include "lua_wrapper.h"

#include <iostream>


static int Debug(lua_State *pState) {
	gLuaManager.GetDebugger()->Debug();

	return 0;
}

void TestDebug() {
	gLuaManager.InitScript();

	RegistFuntion2Lua(Debug);

	gLuaManager.LoadScript(SOURCEPATH, "debug_main.lua");
}

int main() {
	TestDebug();
	return 0;
}

