#include "lua_protobuf.h"
#ifdef USEPROTOBUF
#include "lua_common.h"

using namespace google::protobuf;
using namespace google::protobuf::compiler;

LuaProtobuf::LuaProtobuf()
	: m_sourceTree(new DiskSourceTree())
	, m_importer(NULL)
	, m_factory(NULL) {

}
LuaProtobuf::~LuaProtobuf() {
	if (m_sourceTree) {
		delete m_sourceTree;
		m_sourceTree = NULL;
	}

	if (m_importer) {
		delete m_importer;
		m_importer = NULL;
	}

	if (m_factory) {
		delete m_factory;
		m_factory = NULL;
	}
}


google::protobuf::Message* LuaProtobuf::CreateMessage(const char* typeName) {
	google::protobuf::Message* message = NULL;
	if (m_importer) {
		const google::protobuf::Descriptor* descriptor = m_importer->pool()->FindMessageTypeByName(typeName);
		if (descriptor) {
			const google::protobuf::Message* prototype = m_factory->GetPrototype(descriptor);
			if (prototype) {
				message = prototype->New();
			}
		} else {// 如果在import进来的proto里找不到，就去生成的pool里再找一次 
			const google::protobuf::Descriptor* descriptor =
				google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
			if (descriptor) {
				const google::protobuf::Message* prototype =
					google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
				if (prototype) {
					message = prototype->New();
				}
			}
		}
	} else {
		const google::protobuf::Descriptor* descriptor =
			google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName(typeName);
		if (descriptor) {
			const google::protobuf::Message* prototype =
				google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
			if (prototype) {
				message = prototype->New();
			}
		}
	}
	return message;
}

class __ProtobufErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector {
	virtual void AddError(
		const std::string & filename,
		int line,
		int column,
		const std::string & message) {
		char msg[512] = { 0 };
		snprintf(msg, sizeof(msg), "%s:%d:%d:%s\n", filename.c_str(), line, column, message.c_str());
		std::cerr << msg;
	}
};

void LuaProtobuf::LoadRootProto(const std::string& file, const std::string& diskPath) {
	m_sourceTree->MapPath("", diskPath);

	if (m_importer) {
		delete m_importer;
		m_importer = NULL;
	}
	if (m_factory) {
		delete m_factory;
		m_factory = NULL;
	}

	__ProtobufErrorCollector pbec;
	m_importer = new Importer(m_sourceTree, &pbec);
	m_factory = new DynamicMessageFactory();

	const google::protobuf::FileDescriptor* fileDescriptor = m_importer->Import(file);
	if (!fileDescriptor) {
		char errMsg[256] = { 0 };
		snprintf(errMsg, sizeof(errMsg), "LuaProtobuf::loadRootProto(): import failed!\n");
		std::cerr << errMsg;
	}
}

google::protobuf::Message* LuaProtobuf::LuaToProtobuf(const char* pbName, lua_State* pState, int stackIndex) {

	if (!lua_istable(pState, stackIndex)) {
		char errMsg[256] = { 0 };
		snprintf(errMsg, sizeof(errMsg), "pbtype = %s,there is no table on the lua stack at given index %d, type is %s\n", pbName, stackIndex, lua_typename(pState, lua_type(pState, stackIndex)));
		std::cerr << errMsg;
		return NULL;
	}

	Message* message = LuaProtobuf::GetLuaProtobuf().CreateMessage(pbName);
	if (!message) {
		char errMsg[256] = { 0 };
		snprintf(errMsg, sizeof(errMsg), "cant find message  %s from compiled poll \n", pbName);
		std::cerr << errMsg;
		return NULL;
	}

	const Reflection* reflection = message->GetReflection();
	const Descriptor* descriptor = message->GetDescriptor();

	// 遍历table的所有key， 并且与 protobuf结构相比较。如果require的字段没有赋值， 报错！ 如果找不到字段，报错！
	for (int32_t index = 0; index < descriptor->field_count(); ++index) {
		const FieldDescriptor* fd = descriptor->field(index);
		const string& name = fd->name();

		bool isRequired = fd->is_required();
		bool bReapeted = fd->is_repeated();
		lua_pushstring(pState, name.c_str());
		lua_rawget(pState, stackIndex);

		bool isNil = lua_isnil(pState, -1);

		if (bReapeted) {
			if (isNil) {

				lua_pop(pState, 1);
				continue;
			} else {
				bool isTable = lua_istable(pState, -1);
				if (!isTable) {
					char errMsg[256] = { 0 };
					snprintf(errMsg, sizeof(errMsg), "cant find required repeated field %s\n", name.c_str());
					std::cerr << errMsg;
					return NULL;
				}
			}

			lua_pushnil(pState);
			for (; lua_next(pState, -2) != 0;) {
				switch (fd->cpp_type()) {

				case FieldDescriptor::CPPTYPE_DOUBLE:
				{
					double value = luaL_checknumber(pState, -1);
					reflection->AddDouble(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_FLOAT:
				{
					float value = (float)luaL_checknumber(pState, -1);
					reflection->AddFloat(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_INT64:
				{
					int64_t value = luaL_checkinteger(pState, -1);
					reflection->AddInt64(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_UINT64:
				{
					uint64_t value = luaL_checkinteger(pState, -1);
					reflection->AddUInt64(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
				{
					int32_t value = (int32_t)luaL_checkinteger(pState, -1);
					const EnumDescriptor* enumDescriptor = fd->enum_type();
					const EnumValueDescriptor* valueDescriptor = enumDescriptor->FindValueByNumber(value);
					reflection->AddEnum(message, fd, valueDescriptor);
				}
				break;
				case FieldDescriptor::CPPTYPE_INT32:
				{
					int32_t value = (int32_t)luaL_checkinteger(pState, -1);
					reflection->AddInt32(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_UINT32:
				{
					uint32_t value = (uint32_t)luaL_checkinteger(pState, -1);
					reflection->AddUInt32(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_STRING:
				{
					size_t size = 0;
					const char* value = luaL_checklstring(pState, -1, &size);
					reflection->AddString(message, fd, std::string(value, size));
				}
				break;
				case FieldDescriptor::CPPTYPE_BOOL:
				{
					bool value = lua_toboolean(pState, -1);
					reflection->AddBool(message, fd, value);
				}
				break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
				{

					Message* value = LuaToProtobuf(fd->message_type()->full_name().c_str(), pState, lua_gettop(pState));
					if (!value) {
						char errMsg[256] = { 0 };
						snprintf(errMsg, sizeof(errMsg), "convert to message %s failed whith value %s\n", fd->message_type()->full_name().c_str(), name.c_str());
						std::cerr << errMsg;
						return NULL;
					}
					Message* msg = reflection->AddMessage(message, fd);
					msg->CopyFrom(*value);
					delete value;
				}
				break;
				default:
					break;
				}

				// remove value， keep the key
				lua_pop(pState, 1);
			}
		} else {

			if (isRequired) {
				if (isNil) {
					char errMsg[256] = { 0 };
					snprintf(errMsg, sizeof(errMsg), "cant find required field %s\n", name.c_str());
					std::cerr << errMsg;
					return NULL;
				}
			} else {
				if (isNil) {
					lua_pop(pState, 1);
					continue;
				}
			}

			switch (fd->cpp_type()) {
			case FieldDescriptor::CPPTYPE_DOUBLE:
			{
				double value = luaL_checknumber(pState, -1);
				reflection->SetDouble(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_FLOAT:
			{
				float value = (float)luaL_checknumber(pState, -1);
				reflection->SetFloat(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_INT64:
			{
				int64_t value = luaL_checkinteger(pState, -1);
				reflection->SetInt64(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_UINT64:
			{
				uint64_t value = luaL_checkinteger(pState, -1);
				reflection->SetUInt64(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
			{
				int32_t value = (int32_t)luaL_checkinteger(pState, -1);
				const EnumDescriptor* enumDescriptor = fd->enum_type();
				const EnumValueDescriptor* valueDescriptor = enumDescriptor->FindValueByNumber(value);
				reflection->SetEnum(message, fd, valueDescriptor);
			}
			break;
			case FieldDescriptor::CPPTYPE_INT32:
			{
				int32_t value = (int32_t)luaL_checkinteger(pState, -1);
				reflection->SetInt32(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_UINT32:
			{
				uint32_t value = (uint32_t)luaL_checkinteger(pState, -1);
				reflection->SetUInt32(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_STRING:
			{
				size_t size = 0;
				const char* value = luaL_checklstring(pState, -1, &size);
				reflection->SetString(message, fd, std::string(value, size));
			}
			break;
			case FieldDescriptor::CPPTYPE_BOOL:
			{
				bool value = lua_toboolean(pState, -1);
				reflection->SetBool(message, fd, value);
			}
			break;
			case FieldDescriptor::CPPTYPE_MESSAGE:
			{
				Message* value = LuaToProtobuf(fd->message_type()->full_name().c_str(), pState, lua_gettop(pState));
				if (!value) {
					char errMsg[256] = { 0 };
					snprintf(errMsg, sizeof(errMsg), "convert to message %s failed whith value %s \n", fd->message_type()->full_name().c_str(), name.c_str());
					std::cerr << errMsg;
					return NULL;
				}
				Message* msg = reflection->MutableMessage(message, fd);
				msg->CopyFrom(*value);
				delete value;
			}
			break;
			default:
				break;
			}
		}

		// pop value
		lua_pop(pState, 1);
	}

	return message;
}

void LuaProtobuf::PushProtobufToLuaTable(lua_State* pState, const google::protobuf::Message* message) {
	if (!message) {
		std::cerr << "PushProtobuf2LuaTable failed, message is NULL\n";
		return;
	}
	const Reflection* reflection = message->GetReflection();

	// 顶层table
	lua_newtable(pState);

	const Descriptor* descriptor = message->GetDescriptor();
	for (int32_t index = 0; index < descriptor->field_count(); ++index) {
		const FieldDescriptor* fd = descriptor->field(index);
		const std::string& name = fd->name();

		// key
		lua_pushstring(pState, name.c_str());

		bool bReapeted = fd->is_repeated();

		if (bReapeted) {
			// repeated这层的table
			lua_newtable(pState);
			int size = reflection->FieldSize(*message, fd);
			for (int i = 0; i < size; ++i) {
				char str[32] = { 0 };
				switch (fd->cpp_type()) {
				case FieldDescriptor::CPPTYPE_DOUBLE:
					lua_pushnumber(pState, reflection->GetRepeatedDouble(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_FLOAT:
					lua_pushnumber(pState, (double)reflection->GetRepeatedFloat(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_INT64:

					snprintf(str, sizeof(str), "%lld", (long long)reflection->GetRepeatedInt64(*message, fd, i));
					lua_pushstring(pState, str);
					break;
				case FieldDescriptor::CPPTYPE_UINT64:

					snprintf(str, sizeof(str), "%llu", (unsigned long long)reflection->GetRepeatedUInt64(*message, fd, i));
					lua_pushstring(pState, str);
					break;
				case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
					lua_pushinteger(pState, reflection->GetRepeatedEnum(*message, fd, i)->number());
					break;
				case FieldDescriptor::CPPTYPE_INT32:
					lua_pushinteger(pState, reflection->GetRepeatedInt32(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_UINT32:
					lua_pushinteger(pState, reflection->GetRepeatedUInt32(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_STRING:
				{
					std::string value = reflection->GetRepeatedString(*message, fd, i);
					lua_pushlstring(pState, value.c_str(), value.size());
				}
				break;
				case FieldDescriptor::CPPTYPE_BOOL:
					lua_pushboolean(pState, reflection->GetRepeatedBool(*message, fd, i));
					break;
				case FieldDescriptor::CPPTYPE_MESSAGE:
					PushProtobufToLuaTable(pState, &(reflection->GetRepeatedMessage(*message, fd, i)));
					break;
				default:
					break;
				}

				lua_rawseti(pState, -2, i + 1); // lua's index start at 1
			}

		} else {
			char str[32] = { 0 };
			switch (fd->cpp_type()) {

			case FieldDescriptor::CPPTYPE_DOUBLE:
				lua_pushnumber(pState, reflection->GetDouble(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_FLOAT:
				lua_pushnumber(pState, (double)reflection->GetFloat(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_INT64:

				snprintf(str, sizeof(str), "%lld", (long long)reflection->GetInt64(*message, fd));
				lua_pushstring(pState, str);
				break;
			case FieldDescriptor::CPPTYPE_UINT64:

				snprintf(str, sizeof(str), "%llu", (unsigned long long)reflection->GetUInt64(*message, fd));
				lua_pushstring(pState, str);
				break;
			case FieldDescriptor::CPPTYPE_ENUM: // 与int32一样处理
				lua_pushinteger(pState, (int)reflection->GetEnum(*message, fd)->number());
				break;
			case FieldDescriptor::CPPTYPE_INT32:
				lua_pushinteger(pState, reflection->GetInt32(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_UINT32:
				lua_pushinteger(pState, reflection->GetUInt32(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_STRING:
			{
				std::string value = reflection->GetString(*message, fd);
				lua_pushlstring(pState, value.c_str(), value.size());
			}
			break;
			case FieldDescriptor::CPPTYPE_BOOL:
				lua_pushboolean(pState, reflection->GetBool(*message, fd));
				break;
			case FieldDescriptor::CPPTYPE_MESSAGE:
				PushProtobufToLuaTable(pState, &(reflection->GetMessage(*message, fd)));
				break;
			default:
				break;
			}
		}

		lua_rawset(pState, -3);
	}
}

#endif //USEPROTOBUF