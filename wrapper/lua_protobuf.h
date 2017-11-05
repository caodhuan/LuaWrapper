#pragma once
#ifdef USEPROTOBUF
#include "google/protobuf/compiler/importer.h"
#include "google/protobuf/dynamic_message.h"

#include <string>

struct lua_State;

class LuaProtobuf {
public:
	LuaProtobuf();
	~LuaProtobuf();

	static LuaProtobuf& GetLuaProtobuf() {
		static LuaProtobuf lp;
		return lp;
	}

public:
	// 如果import存在，就去import里找
	// 否则， 尝试去静态生成的里面去找
	google::protobuf::Message* CreateMessage(const char* typeName);

	// 动态的proto
	void LoadRootProto(const std::string& file, const std::string& diskPath);

	// 将lua的table转成protobuf，外层负责delete 返回的指针
	// stackIndex 欲转换的table的栈的索引
	static google::protobuf::Message*  LuaToProtobuf(const char* pbName, lua_State* pState, int stackIndex);

	// 将一个Message转换成一个lua table放入栈顶
	static void PushProtobufToLuaTable(lua_State* pState, const google::protobuf::Message* message);

private:
	google::protobuf::compiler::DiskSourceTree* m_sourceTree;
	google::protobuf::compiler::Importer* m_importer;
	google::protobuf::DynamicMessageFactory* m_factory;
};

#endif // USEPROTOBUF