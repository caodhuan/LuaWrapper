#pragma once


template<class T>
struct LuaResultTraits {};

template<>
struct LuaResultTraits<unsigned long> {
	typedef unsigned  long type;
};

template<>
struct LuaResultTraits<unsigned long long> {
	typedef unsigned long long type;
};

template<>
struct LuaResultTraits<int> {
	typedef	int type;
};

template<>
struct LuaResultTraits<double> {
	typedef	double type;
};

template<>
struct LuaResultTraits<void> {
	typedef	void type;
};

template<>
struct LuaResultTraits<char*> {
	typedef	const char* type;
};

template<>
struct LuaResultTraits<const char*> {
	typedef	const char* type;
};

template<>
struct LuaResultTraits<bool> {
	typedef	bool type;
};

template<>
struct LuaResultTraits<float> {
	typedef	float type;
};