#pragma once
#include "lua_manager.h"
#include "lua_caller.h"
#include "lua_result.h"


static void GetArgs(LuaCaller& caller) {
	return;
}

template<typename T, typename ... Arguments>
static void GetArgs(LuaCaller& caller, T t, Arguments&& ... params) {
	caller += t;
	GetArgs(caller, std::forward<Arguments>(params)...);
}

template<typename T, typename TFunction, typename ... Arguments>
static T CallLuaFunction(TFunction fun, Arguments&& ... params) {
	LuaCaller caller(LuaManager::GetLuaManager().GetLuaState(), fun);
	GetArgs(caller, std::forward<Arguments>(params)...);

	LuaResult result;
	caller.Call(&result);

	return result.GetResult<T>();
}

template<typename TFunction, typename ... Arguments>
static void CallLuaFunction(TFunction fun, Arguments&& ... params) {
	LuaCaller caller(LuaManager::GetLuaManager().GetLuaState(), fun);
	GetArgs(caller, std::forward<Arguments>(params)...);

	caller.Call();
}