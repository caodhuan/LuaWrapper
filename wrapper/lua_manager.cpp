#include "lua_manager.h"
#include "lua_common.h"
#include "lua_caller.h"

#include <string>
#include <sstream>
#include <stdint.h>
#include "lua_debugger.h"

static const char* funTable = "__SCRIPT__FUNCTION__";
static ScriptHandler globalhandler = 0;

LuaManager::LuaManager()
	: m_pState(NULL)
	, m_debuger(NULL)
	, m_logFunction(nullptr)
	, m_errorFunction(nullptr) {
}

LuaManager::~LuaManager() {

	if (m_pState) {
		lua_close(m_pState);
		m_pState = NULL;
	}
}

static int Panic(lua_State* pState) {
	const char *msg = lua_tostring(pState, -1);
	if (gLuaManager.GetErrorFunction()) {
		printf("%s", msg); // 显示在控制台提示一下
		gLuaManager.TryError(msg);
	} else {
		printf("%s", msg);
	}

	return 0;
}

void LuaManager::InitScript() {
	if (m_pState) {
		return;
	}
	m_pState = luaL_newstate();

	lua_atpanic(m_pState, Panic);

	luaL_openlibs(m_pState);

	// 初始化handler表
	lua_pushstring(m_pState, funTable); //key
	lua_newtable(m_pState);				// handler table
	lua_rawset(m_pState, LUA_REGISTRYINDEX); // regist[funTable] = {}

	m_debuger = new LuaDebugger(m_pState);
}

void LuaManager::AddSearchPath(const char* path) {

	char strPath[1024] = { 0 };
	snprintf(strPath, sizeof(strPath), "local path = string.match([[%s]],[[(.*)/[^/]*$]])\n package.path = package.path .. [[;]] .. path .. [[/?.lua;]] .. path .. [[/?/init.lua]]\n", path);
	ExecuteString(strPath);
}

void LuaManager::LoadScript(const char* path, const char* startScriptFile) {
	AddSearchPath(path);
	ExecuteFile((std::string(path) + startScriptFile).c_str());
}

void LuaManager::RegistFunction2Lua(const char* funName, lua_CFunction fun) {
	lua_pushcfunction(m_pState, fun);
	lua_setglobal(m_pState, funName);
}

bool LuaManager::ExecuteString(const char* str) {
	if (luaL_dostring(m_pState, str)) {
		LuaCaller::Traceback(m_pState);

		return false;
	}
	return true;
}

bool LuaManager::ExecuteFile(const char* path) {
	if (luaL_dofile(m_pState, path)) {
		LuaCaller::Traceback(m_pState);

		return false;
	}
	return true;
}

void LuaManager::BeginGetTable(const char* tableName) {
	lua_getglobal(m_pState, tableName);
}

void LuaManager::EndGetTable() {
	lua_pop(m_pState, 1);
}

void LuaManager::BeginGetTableTable(const char* tableName) {
	lua_pushstring(m_pState, tableName);
	lua_gettable(m_pState, -2);
}

void LuaManager::EndGetTableTable() {
	lua_pop(m_pState, 1);
}

ScriptHandler LuaManager::LuaToScriptHandler(lua_State* pState, int idx) {
	if (pState != m_pState) {
		TryLog("multi lua vm attached!");

		return 0;
	}

	if (!lua_isfunction(pState, idx)) {
		// CTODO 打印出lua堆栈
		TryLog("to script handler failed, function excepted!");
		return 0;
	}

	ScriptHandler handler = globalhandler++;

	PushFunScriptTable();

	lua_pushnumber(m_pState, handler);
	lua_pushvalue(m_pState, idx);
	lua_rawset(m_pState, -3);  //  -1: luavalue, -2 handler, -3 funtable

	lua_pop(m_pState, 1); // pop funtable

	return handler;
}

void LuaManager::RemoveScriptHandler(ScriptHandler handler) {
	PushFunScriptTable();
	lua_pushnumber(m_pState, handler);
	lua_pushnil(m_pState);
	lua_rawset(m_pState, -3);

	lua_pop(m_pState, 1); // pop funtable
}

LuaDebugger* LuaManager::GetDebugger() {
	return m_debuger;
}

void LuaManager::SetLogFunction(LOGFUNCTION function) {
	m_logFunction = function;
}

void LuaManager::SetErrorFunction(LOGFUNCTION function) {
	m_errorFunction = function;
}

void LuaManager::TryLog(const char* msg) {
	if (!m_logFunction) {
		printf("%s\n", msg);
		return;
	}
	DoLogMessage(m_logFunction, msg);
}

void LuaManager::TryError(const char* msg) {
	if (!m_errorFunction) {
		printf("%s\n", msg);
		return;
	}
	DoLogMessage(m_errorFunction, msg);
}

void LuaManager::GenerateLuaError(const char* errorMsg, ...) {
	va_list args;
	char msg[256];
	va_start(args, errorMsg);
	vsnprintf(msg, sizeof(msg) / sizeof(*msg), errorMsg, args);

	va_end(args);

	lua_pushstring(m_pState, msg);

	lua_error(m_pState);
}


bool LuaManager::ReloadFile(const char* filePath) {
	bool failed = luaL_dofile(m_pState, filePath);

	if (failed) {
		TryError(lua_tostring(m_pState, -1));
	}

	return !failed;
}

void LuaManager::PushFunScriptTable() {
	lua_pushstring(m_pState, funTable);
	lua_rawget(m_pState, LUA_REGISTRYINDEX);
}

bool LuaManager::DoGetGlobalVarible(const char* varName, bool) {
	lua_getglobal(m_pState, varName);
#pragma warning(disable:4800)
	return static_cast<bool>(lua_toboolean(m_pState, -1));
#pragma warning(default:4800)
}

int LuaManager::DoGetGlobalVarible(const char* varName, int) {
	lua_getglobal(m_pState, varName);
	return static_cast<int>(luaL_checkinteger(m_pState, -1));
}

const char* LuaManager::DoGetGlobalVarible(const char* varName, const char*) {
	lua_getglobal(m_pState, varName);
	return luaL_checkstring(m_pState, -1);
}

bool LuaManager::DoGetTableVarible(const char* varName, bool) {
	lua_pushstring(m_pState, varName);
	lua_gettable(m_pState, -2);

#pragma warning(disable:4800)    
	bool result = static_cast<bool>(lua_toboolean(m_pState, -1));
#pragma warning(default:4800)
	lua_pop(m_pState, 1);

	return result;

}

int LuaManager::DoGetTableVarible(const char* varName, int) {
	lua_pushstring(m_pState, varName);
	lua_gettable(m_pState, -2);

	int result = static_cast<int>(luaL_checkinteger(m_pState, -1));
	lua_pop(m_pState, 1);

	return result;
}

const char* LuaManager::DoGetTableVarible(const char* varName, const char*) {
	lua_pushstring(m_pState, varName);
	lua_gettable(m_pState, -2);

	const char* result = luaL_checkstring(m_pState, -1);

	lua_pop(m_pState, 1);

	return result;
}

void LuaManager::DoLogMessage(LOGFUNCTION function, const char* msg) {
	lua_Debug info;
	int depth = 0;
	while (lua_getstack(m_pState, depth, &info)) {

		lua_getinfo(m_pState, "S", &info);
		lua_getinfo(m_pState, "n", &info);
		lua_getinfo(m_pState, "l", &info);

		if (info.source[0] == '@') {
			function(&info.source[1], info.currentline, msg);
			return;
		}

		++depth;
	}
	if (depth == 0) {
		// 说明没有堆栈
		function("trunk", 0, msg);
	}
}
