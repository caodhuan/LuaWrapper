#include "common.h"
#include <iostream>
#include "lua_wrapper.h"

using namespace std;

void TestNoArg() {
	LuaCaller caller(gLuaManager.GetLuaState(), "fNoArg");

	caller.Call(NULL);
}

void TestWithArgs() {
	LuaCaller caller(gLuaManager.GetLuaState(), "fTwoArg");
	caller += 1, "msg from cpp";
	caller.Call(NULL);
}

void TestWithReturn() {
	LuaResult result;
	LuaCaller caller(gLuaManager.GetLuaState(), "fAdd");
	caller += 1, 2;
	caller.Call(&result);

	int count =  result.GetResult<int>();
	cout << "result = " << count << endl;
}

void TestWithMutiReturn() {
	LuaResult result;
	LuaCaller caller(gLuaManager.GetLuaState(), "fReturnTwo");
	caller += 1, 2;
	caller.Call(&result);

	int index = 1;
	while(result.GetResultCount())
	{
		int count = result.GetResult<int>();
		cout << "result " << index++ << "  = " << count << endl;
	}

}

static int CppAddAndSub(lua_State* pState) {
	int first = luaL_checkinteger(pState, 1);
	int second = luaL_checkinteger(pState, 2);

	lua_pushinteger(pState, first + second);
	lua_pushinteger(pState, first - second);

	return 2;
}


int main() {
	gLuaManager.InitScript();

	RegistFuntion2Lua(CppAddAndSub);

	gLuaManager.LoadScript(SOURCEPATH, "main.lua");

	TestNoArg();
	TestWithArgs();
	TestWithReturn();
	TestWithMutiReturn();


	cout << "encapsulate with template " << endl;

	CallLuaFunction("fNoArg");
	CallLuaFunction("fTwoArg", 1, 2);
	int r = CallLuaFunction<int>("fAdd", 1, 100);
	cout << "fAdd = " << r << endl;


	gLuaManager.ExecuteString("print(debug.traceback(\"test\"))");

	return 0;
}

