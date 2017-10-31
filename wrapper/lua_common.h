#pragma once
#include <lua.hpp>

typedef int ScriptHandler;

// lualib的总的头文件
#include "lua_caller.h"
#include "lua_manager.h"
#include "lua_result.h"



inline void GetArgs(LuaCaller& caller) {
	return;
}

template<typename T, typename ... Arguments>
void GetArgs(LuaCaller& caller, T t, Arguments&& ... params)
{
	caller += t;
	GetArgs(caller, std::forward<Arguments>(params)...);
}

template<typename T, typename TFunction, typename ... Arguments>
T CallLuaFunction(TFunction fun, Arguments&& ... params) {
	LuaCaller caller(gLuaManager.GetLuaState(), fun);
	GetArgs(caller, std::forward<Arguments>(params)...);

	LuaResult result;
	caller.Call(&result);

	return result.GetResult<T>();
}

template<typename TFunction, typename ... Arguments>
void CallLuaFunction(TFunction fun, Arguments&& ... params) {
	LuaCaller caller(gLuaManager.GetLuaState(), fun);
	GetArgs(caller, std::forward<Arguments>(params)...);

	caller.Call();
}