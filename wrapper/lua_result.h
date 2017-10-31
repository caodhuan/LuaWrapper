#pragma once
#include "lua_common.h"

class LuaResult {
	friend LuaCaller;
public:
	LuaResult();
	~LuaResult();

public:

	template<typename T>
	T GetResult() {
		typedef typename LuaArg<T>::type ValueType;
		ValueType value = ValueType();

		return Result(value);
	}

	int GetResultCount() {
		return m_resultCount;
	}

private:
	void SetLuaState(lua_State* pState);

	void SetResultCount(int count);

	bool Result(bool);
	int Result(int);
	unsigned int Result(unsigned int);
	double Result(double);
	const char* Result(const char*);
	long Result(long);
	unsigned long Result(unsigned long);
	long long Result(long long);

private:
	lua_State* m_pState;
	int m_resultCount;

};