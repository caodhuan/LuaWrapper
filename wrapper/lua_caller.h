#pragma once

#include "lua_common.h"

#include <string>

class LuaResult;

class LuaCaller {
	friend class LuaManager;
public:
	LuaCaller() {};

	LuaCaller(lua_State*, ScriptHandler);
	LuaCaller(lua_State*, const char*);
	~LuaCaller();
	static int	Traceback(lua_State *L);

	// 增加多少（可以为负数）个参数索引
	// 除非知道自己在干嘛， 不然别使用这个方法！！！
	void IncreaseArgNum(int value);
private:
	LuaCaller(lua_State*);
	LuaCaller(const LuaCaller&);
	LuaCaller& operator=(const LuaCaller&);

public:

	// 是否有这个函数
	bool IsOK();

	template<typename T>
	LuaCaller& operator,(T t) {
		return this->Arg(t);
	}

	template<typename T>
	LuaCaller& operator=(T t) {
		return this->Arg(t);
	}

	template<typename T>
	LuaCaller& operator+=(T t) {
		return this->Arg(t);
	}

	template<typename T>
	LuaCaller& operator<<(T t) {
		return this->Arg(t);
	}


	bool Call(LuaResult* result = NULL);

private:

	LuaCaller& Arg(int value);
	LuaCaller& Arg(const char* value);
	LuaCaller& Arg(double value);
	LuaCaller& Arg(bool value);
	LuaCaller& Arg(unsigned int value);
	LuaCaller& Arg(unsigned long value);
	LuaCaller& Arg(unsigned long long value);
	LuaCaller& Arg(const std::string& value);
	LuaCaller& Arg(long int value);

private:
	lua_State* m_pState;
	int m_numArgs;
	int m_tb;
	bool m_called;
};