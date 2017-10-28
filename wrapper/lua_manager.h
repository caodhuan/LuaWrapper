#pragma once
#include "lua_common.h"
#include "lua_arg.hpp"

#include <stdint.h>
#include <functional>

#define gLuaManager LuaManager::GetLuaManager()

#define RegistFuntion2Lua(fun) gLuaManager.RegistFunction2Lua(#fun, fun)


class LuaCaller;

typedef std::function<void(const char* fileName, uint32_t lineNum, const char* msg)> LOGFUNCTION;

class LuaManager {
	friend LuaCaller;

public:
	~LuaManager();

public:
	lua_State* GetLuaState() {
		return m_pState;
	}
	
	static LuaManager& GetLuaManager() {
		static LuaManager sc;
		return sc;
	}
public:
	void InitScript();

    void AddSearchPath(const char* path);
	
	// 加载lua脚本环境
	// path  路径
	// startScript 启动脚本文件
	void LoadScript(const char* path, const char* startScriptFile);

	// 注册一个c函数到lua
	void RegistFunction2Lua(const char* funName, lua_CFunction fun);

    bool ExecuteString(const char* str);

    bool ExecuteFile(const char* path);

    // 可以嵌套, 用下面的GetTableTable。
    void BeginGetTable(const char* tableName);
    void EndGetTable();

    void BeginGetTableTable(const char* tableName);
    void EndGetTableTable();

    template<typename T>
    T GetTableVarible(const char* varName) {
        typedef typename LuaArg<T>::type ValueType;

        ValueType value = ValueType();

        return DoGetTableVarible(varName, value);
    }

    template<typename T>
    T GetGlobalVarible(const char* varName) {
        typedef typename LuaArg<T>::type ValueType;

        ValueType value = ValueType();
        return DoGetGlobalVarible(varName, value);
    }

	// 将lua传过的回调函数转变成handler
	// lua_State*， 这里是为了与lua的接口保持一致
	// idx， 你懂的
	ScriptHandler lua_toScriptHandler(lua_State*, int idx);

	void RemoveScriptHandler(ScriptHandler handler);
    
    void SetLogFunction(LOGFUNCTION function);

    LOGFUNCTION GetLogFunction() {
        return m_logFunction;
    }

    void SetErrorFunction(LOGFUNCTION function);

    LOGFUNCTION GetErrorFunction() {
        return m_errorFunction;
    }
	

    void GenerateLuaError(const char* errorMsg, ...);

	// 更新指定脚本
	bool ReloadFile(const char* filePath);

	// lua用的用来打log的， 除非知道自己在干嘛！！！不然别用！！
	void TryLog(const char* msg);
	void TryError(const char* msg);
private:
	// 将lua回调表放入栈顶
	void PushFunScriptTable();

private:
    bool DoGetGlobalVarible(const char* varName, bool);
    int DoGetGlobalVarible(const char* varName, int);
    const char* DoGetGlobalVarible(const char* varName, const char*);

    bool DoGetTableVarible(const char* varName, bool);
    int DoGetTableVarible(const char* varName, int);
    const char* DoGetTableVarible(const char* varName, const char*);

	void DoLogMessage(LOGFUNCTION function, const char* msg);
private:
	lua_State* m_pState;
    LOGFUNCTION  m_logFunction;
    LOGFUNCTION  m_errorFunction;
private:
	LuaManager();
	LuaManager(const LuaManager&);
	LuaManager& operator=(const LuaManager&);
};