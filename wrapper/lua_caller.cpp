#include "lua_caller.h"
#include "lua_result.h"
#include "lua_manager.h"

#include <string>

const static int weirdPos = 20;

LuaCaller::LuaCaller(lua_State* pState, const char* funName)
	: LuaCaller(pState) {
	std::string fun = funName;
	size_t index = fun.find('.');
	if (index != std::string::npos) {
		std::string table = fun.substr(0, index);
		std::string realFun = fun.substr(index + 1, fun.length() - 1);
		lua_getglobal(m_pState, table.c_str());

		if (lua_istable(m_pState, -1)) {
			lua_pushstring(m_pState, realFun.c_str());
			lua_rawget(m_pState, -2);

			lua_remove(m_pState, -2); // pop掉 funName占的栈
		}

	} else {
		lua_getglobal(m_pState, funName);
	}
}

LuaCaller::LuaCaller(lua_State* pState, ScriptHandler handler)
	: LuaCaller(pState) {
	gLuaManager.PushFunScriptTable();

	lua_pushnumber(m_pState, handler);
	lua_rawget(m_pState, -2);
}

LuaCaller::LuaCaller(lua_State* pState)
	: m_pState(pState)
	, m_numArgs(0)
	, m_called(false) {
	lua_pushcfunction(m_pState, Traceback);
	m_tb = lua_gettop(m_pState);

	if (m_tb >= weirdPos) {
		gLuaManager.TryError("lua 堆栈上caller有异常！！！");
	}
}

LuaCaller::~LuaCaller() {
	lua_settop(m_pState, m_tb - 1);
}

int LuaCaller::Traceback(lua_State *pState) {
	const char *msg = lua_tostring(pState, -1);

	if (msg) {
		gLuaManager.TryError(msg);
	}


	return 0;
}

void LuaCaller::IncreaseArgNum(int value) {
	m_numArgs += value;
}

LuaCaller& LuaCaller::Arg(int value) {
	lua_pushinteger(m_pState, value);
	++m_numArgs;
	return *this;
}

LuaCaller& LuaCaller::Arg(const char* value) {
	lua_pushstring(m_pState, value);
	++m_numArgs;
	return *this;
}

LuaCaller& LuaCaller::Arg(double value) {
	lua_pushnumber(m_pState, value);
	++m_numArgs;
	return *this;
}

LuaCaller& LuaCaller::Arg(bool value) {
	lua_pushboolean(m_pState, value);
	++m_numArgs;
	return *this;
}

LuaCaller& LuaCaller::Arg(unsigned long value) {

	++m_numArgs;
	lua_pushinteger(m_pState, value);

	return *this;
}

LuaCaller& LuaCaller::Arg(unsigned long long value) {

	++m_numArgs;
	lua_pushinteger(m_pState, value);

	return *this;
}

LuaCaller& LuaCaller::Arg(unsigned int value) {
	lua_pushinteger(m_pState, value);
	++m_numArgs;
	return *this;
}

LuaCaller& LuaCaller::Arg(long int value) {
	lua_pushinteger(m_pState, value);
	++m_numArgs;
	return *this;
}

LuaCaller& LuaCaller::Arg(const std::string& value) {
	lua_pushstring(m_pState, value.c_str());
	++m_numArgs;
	return *this;
}

bool LuaCaller::IsOK() {
	return lua_isfunction(m_pState, -1) || lua_iscfunction(m_pState, -1);
}

bool LuaCaller::Call(LuaResult* result) {
	if (m_called) {
		// CTODO: log日志
		return false;
	} else {
		m_called = true;
	}

	if (lua_pcall(m_pState, m_numArgs, LUA_MULTRET, m_tb)) {
		return false;
	}

	if (result) {
		int resultCount = lua_gettop(m_pState) - m_tb;

		result->SetResultCount(resultCount);
		result->SetLuaState(m_pState);
	}

	return true;
}