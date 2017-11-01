#pragma once

#include <string>
#include <mutex>

struct lua_State;

class LuaProtobuf
{
public:
	LuaProtobuf();
	~LuaProtobuf();

public:
	
private:
	// 动态proto部分


private:
	std::mutex m_mutex;
};

