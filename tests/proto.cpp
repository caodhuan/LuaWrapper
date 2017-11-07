#include "common.h"
#include "lua_wrapper.h"

#include "google/protobuf/message.h"

#include <iostream>

using namespace google::protobuf;

void TestCreate() {
	LuaProtobuf::GetLuaProtobuf().LoadRootProto(PROTOFILE, PROTOPATH);

	Message* msg = LuaProtobuf::GetLuaProtobuf().CreateMessage("tTestProto");

	if (!msg)
	{
		std::cerr << "创建protobuf类型:" << "tTestProto" << "失败" << std::endl;
	}
	else
	{
		std::cout << "创建protobuf类型:" << "tTestProto" << "成功" << std::endl;
	}

	delete msg;
	msg = NULL;
}


static int UpdateRootProto(lua_State* pState) {
	const char* fileName = luaL_checkstring(pState, 1);

	const char* filePath = NULL;
	if (lua_gettop(pState) > 1)
	{
		filePath = luaL_checkstring(pState, 2);
	}

	LuaProtobuf::GetLuaProtobuf().LoadRootProto(fileName, filePath == NULL ? SOURCEPATH : filePath);

	lua_pushboolean(pState, true);

	return 1;
}

static int PBEncode(lua_State* pState) {
	const char* pbName = luaL_checkstring(pState, 1);

	google::protobuf::Message* message = LuaProtobuf::GetLuaProtobuf().LuaToProtobuf(pbName, pState, 2);

	if (message)
	{
		std::string b;
		message->SerializeToString(&b);
		lua_pushlstring(pState, b.c_str(), b.size());

		delete message;
		message = NULL;
		return 1;
	}
	else
	{
		std::cerr << "转换pb失败" << pbName << std::endl;
	}

	return 0;
}

static int PBDecode(lua_State* pState) {
	const char* pbName = luaL_checkstring(pState, 1);
	size_t size = 0;

	const char* msg = luaL_checklstring(pState, 2, &size);
	std::string parseString;
	parseString.assign(msg, size);

	google::protobuf::Message*  pbMsg = LuaProtobuf::GetLuaProtobuf().CreateMessage(pbName);

	if (pbMsg)
	{
		if (pbMsg->ParseFromString(parseString))
		{
			LuaProtobuf::GetLuaProtobuf().PushProtobufToLuaTable(pState, pbMsg);
			return 1;
		}
		else
		{
			std::cerr << "parse failed name =" << pbName << std::endl;
		}

	}
	else
	{
		std::cerr << "cant create pb type. name = " << pbName << std::endl;
	}

	return 0;
}

static void RegisterProto2Lua() {

	RegistFuntion2Lua(UpdateRootProto);
	RegistFuntion2Lua(PBEncode);
	RegistFuntion2Lua(PBDecode);
}

void TestLuaProto() {

	gLuaManager.InitScript();

	RegisterProto2Lua();

	gLuaManager.LoadScript(SOURCEPATH, "proto_main.lua");
}

int main() {
	TestCreate();

	TestLuaProto();
	return 0;
}

