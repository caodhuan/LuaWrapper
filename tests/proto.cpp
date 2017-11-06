#include "common.h"
#include <iostream>
#include "lua_wrapper.h"
#include "google/protobuf/message.h"

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

int main() {
	TestCreate();
	return 0;
}

