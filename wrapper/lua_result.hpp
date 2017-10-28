#pragma once


template<class T>
struct LuaResult {};

template<>
struct LuaResult<unsigned long> {
	typedef unsigned  long type;
};

template<>
struct LuaResult<unsigned long long> {
	typedef unsigned long long type;
};

template<>
struct LuaResult<int> {
	typedef	int type;
};

template<>
struct LuaResult<double> {
	typedef	double type;
};

template<>
struct LuaResult<void> {
	typedef	void type;
};

template<>
struct LuaResult<char*> {
	typedef	const char* type;
};

template<>
struct LuaResult<const char*> {
	typedef	const char* type;
};

template<>
struct LuaResult<bool> {
	typedef	bool type;
};

template<>
struct LuaResult<float> {
	typedef	float type;
};