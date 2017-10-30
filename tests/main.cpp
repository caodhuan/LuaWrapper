#include "common.h"
#include "lua_common.h"
#include <iostream>

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

int main() {
	gLuaManager.InitScript();

	gLuaManager.LoadScript(SOURCEPATH, "main.lua");

	TestNoArg();
	TestWithArgs();
	TestWithReturn();
	TestWithMutiReturn();
	return 0;
}

