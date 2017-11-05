#pragma once
#include "lua_common.h"

#include <vector>

class LuaDebugger {
	friend class LuaManager;
public:
	~LuaDebugger();
public:

	// 支持 打印变量
	// 支持进入任意堆栈
	void Debug();

public:
	static void PrintStack(lua_State* pState);

private:
	void PrintVariable(const char* variableName, int stackLevel);
	const char* ReadWord(bool bNewLine = false);

	bool PrintFrame(int level);

	void PrintLua(std::vector<std::string>& fields);

	void StepInto();

	static void HooK(lua_State* pState, lua_Debug *ld);

	void HookCallback(lua_State* pState, lua_Debug *ld);

	int GetStackdepth();

	// 打印出上下文的代码
	void PrintContextLine();
private:

	LuaDebugger(lua_State* pState);
	LuaDebugger(const LuaDebugger&);
	LuaDebugger& operator=(const LuaDebugger&);

private:
	lua_State* m_pState;
	char m_input[1024];
	char* m_pBuffer;
	bool m_isDebugging;
};