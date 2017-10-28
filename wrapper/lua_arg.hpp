#pragma once


template<class T>
struct LuaArg {};

template<>
struct LuaArg<unsigned long> {
	typedef unsigned  long type;
};

template<>
struct LuaArg<unsigned long long> {
	typedef unsigned long long type;
};

template<>
struct LuaArg<int> {
	typedef	int type;
};

template<>
struct LuaArg<double> {
	typedef	double type;
};

template<>
struct LuaArg<void> {
	typedef	void type;
};

template<>
struct LuaArg<char*> {
	typedef	const char* type;
};

template<>
struct LuaArg<const char*> {
	typedef	const char* type;
};

template<>
struct LuaArg<bool> {
	typedef	bool type;
};

template<>
struct LuaArg<float> {
	typedef	float type;
};