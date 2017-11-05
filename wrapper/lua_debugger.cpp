#include "lua_debugger.h"
#include "lua_manager.h"


#include <fstream>
#include <sstream>


LuaDebugger::LuaDebugger(lua_State* pState)
	: m_pState(pState)
	, m_pBuffer(NULL)
	, m_isDebugging(false) {

}

LuaDebugger::~LuaDebugger() {

}
// 显示出一行脚本
static bool ShowScriptLine(const char* fullFilePath, int lineNumber, int contextLine = 3) {
	if (!fullFilePath) {
		return false;
	}
	std::ifstream pFile(fullFilePath);
	if (!pFile.is_open()) {
		return false;
	}

	for (int i = 1; i < lineNumber; ++i) {
		char content[256] = { 0 };
		pFile.getline(content, 256);

		if (!content) {
			printf("out of file line !\n"); // 文件读完了
			return false;
		}
	}

	for (int i = 0; i < contextLine; ++i) {

		char content[256] = { 0 };
		pFile.getline(content, 256);
		printf("%d\t%s\n", lineNumber + i, content);
		if (!content) {
			return true;
		}
	}
	return true;
}

void LuaDebugger::Debug() {
	m_isDebugging = true;
	int currentStacklevel = 1;
	// p variableName
	// f stacklevel
	// bt backtrace
	// n 单步调试
	// c 跳过
	while (true) {
		const char* command = ReadWord(true);
		if (!strcmp(command, "help")) {
			printf(
				"p variableName  打印变量名字\n" \
				"f stacklevel    步入堆栈\n"      \
				"bt              显示调用堆栈\n"  \
				"n               单步调试\n"    \
				"c               跳过\n"  \
				"l               显示当前上下文代码\n");
		} else if (!strcmp(command, "p")) {
			const char* variable = ReadWord();
			if (!variable) {
				printf("unknow variable name\n");
				continue;
			}

			PrintVariable(variable, currentStacklevel);

		} else if (!strcmp(command, "f")) {
			const char* level = ReadWord();
			bool validStackLevel = true;
			if (level && isdigit(*level)) {
				currentStacklevel = atoi(level);
				lua_Debug ld;
				if (lua_getstack(m_pState, currentStacklevel, &ld)) {

				} else {
					validStackLevel = false;
				}
			} else {
				validStackLevel = false;
			}

			if (!validStackLevel) {
				printf("invalid stack level\n");
				continue;
			}

			PrintFrame(currentStacklevel);
		} else if (!strcmp(command, "bt")) {

			this->PrintStack(m_pState);
		} else if (!strcmp(command, "c")) {
			break;
		} else if (!strcmp(command, "n") || !strcmp(command, "next")) {
			StepInto();
			break;
		} else if (!strcmp(command, "l")) {
			PrintContextLine();
		} else {
			printf("Invalid command!\n");
		}

	}

	m_isDebugging = false;
}

void LuaDebugger::PrintStack(lua_State* pState) {
	std::stringstream stackInfo;

	lua_Debug info;
	int depth = 0;
	while (lua_getstack(pState, depth, &info)) {
		stackInfo << depth << " ";

		lua_getinfo(pState, "S", &info);
		lua_getinfo(pState, "n", &info);
		lua_getinfo(pState, "l", &info);

		if (info.name) {
			stackInfo << info.name;
		} else {
			stackInfo << "(trunk)";
		}

		stackInfo << " ";

		stackInfo << info.source;

		if (info.source[0] == '@') {
			stackInfo << " :" << info.currentline;
		}

		stackInfo << "\n";
		++depth;
	}

	printf("%s", stackInfo.str().c_str());
	gLuaManager.TryError(stackInfo.str().c_str());
}

void LuaDebugger::PrintVariable(const char* variableName, int stackLevel) {
	lua_Debug ld;
	std::vector<char> tmpVariableName(strlen(variableName) + 1, 0);
	memcpy(&tmpVariableName[0], variableName, strlen(variableName));

	if (lua_getstack(m_pState, stackLevel, &ld)) {
		// 有可能是一个table的 ... 的字段
		std::vector<std::string> listVariable;
		int tmpIndex = 0;
		for (int i = 0; tmpVariableName[i]; ++i) {
			if (tmpVariableName[i] == '.') {
				tmpVariableName[i] = 0;
				listVariable.push_back(&tmpVariableName[tmpIndex]);
				tmpIndex = i + 1;
			}
		}
		// 最后一个变量的名字
		listVariable.push_back(&tmpVariableName[tmpIndex]);

		std::string firstName = *listVariable.begin();

		listVariable.erase(listVariable.begin());

		int n = 1;
		bool find = false;
		const char* name = NULL;
		// local变量
		while (!find && (name = lua_getlocal(m_pState, &ld, n++)) != NULL) {
			find = firstName == name;
			if (find) {
				PrintLua(listVariable);
			} else {
				lua_pop(m_pState, 1);
			}
		}
		// global 变量
		if (!find) {
			lua_getglobal(m_pState, firstName.c_str());
			if (lua_type(m_pState, -1) != LUA_TNIL) {
				find = true;
				// 打印变量
				PrintLua(listVariable);
			} else {
				find = false;
				lua_pop(m_pState, 1);
			}
		}
		// upvalue？


		// 直接在lua环境中打印
		if (!find) {
			char luaScript[1024] = { 0 };
			sprintf(luaScript, "print(%s)", variableName);
			gLuaManager.ExecuteString(luaScript);
		}

	} else {
		printf("invalid stack level!\n");
	}
}

const char* LuaDebugger::ReadWord(bool bNewLine /*= false*/) {
	if (bNewLine) {
		m_pBuffer = NULL;
	}

	if (!m_pBuffer) {
		if (!bNewLine) {
			return NULL;
		}

		printf("(debug) ");
		if (!fgets(m_input, sizeof(m_input), stdin)) {
			return NULL;
		}
		m_pBuffer = m_input;
	}

	while (*m_pBuffer == ' ' || *m_pBuffer == '\t') {
		m_pBuffer++;
	}

	char* pCurrent = m_pBuffer;
	while (*m_pBuffer != ' ' && *m_pBuffer != '\t' && *m_pBuffer != '\n' && *m_pBuffer != '\r' && *m_pBuffer != 0) {
		m_pBuffer++;
	}

	while (*m_pBuffer == ' ' || *m_pBuffer == '\t') {
		*m_pBuffer = 0;
		m_pBuffer++;
	}

	if (*m_pBuffer == 0 || *m_pBuffer == '\n' || *m_pBuffer == '\r') {
		*m_pBuffer = 0;
		m_pBuffer = NULL;
	}

	return pCurrent;
}

bool LuaDebugger::PrintFrame(int level) {
	lua_Debug info;

	if (lua_getstack(m_pState, level, &info)) {
		std::stringstream stackInfo;
		stackInfo << level << " ";
		lua_getinfo(m_pState, "S", &info);
		lua_getinfo(m_pState, "n", &info);
		lua_getinfo(m_pState, "l", &info);

		if (info.name) {
			stackInfo << info.name;
		} else {
			stackInfo << "(trunk)";
		}

		stackInfo << " ";

		stackInfo << info.source;

		if (info.source[0] == '@') {
			stackInfo << " :" << info.currentline;
		}

		stackInfo << "\n";

		printf("%s", stackInfo.str().c_str());
	} else {
		return false;
	}

	return true;
}

void LuaDebugger::PrintLua(std::vector<std::string>& fields) {
	lua_getglobal(m_pState, "DebugPrint");
	lua_pushvalue(m_pState, -2);
	for (std::vector<std::string>::const_iterator it = fields.begin(); it != fields.end(); it++) {
		const char* str = it->c_str();
		if (str[0] >= '0' && str[0] <= '9')
			lua_pushnumber(m_pState, atof(str));
		else
			lua_pushstring(m_pState, str);
	}
	lua_pcall(m_pState, 1 + fields.size(), 0, 0);
}

void LuaDebugger::StepInto() {
	lua_sethook(m_pState, LuaDebugger::HooK, LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE, 0);
}

void LuaDebugger::HooK(lua_State* pState, lua_Debug *ld) {
	gLuaManager.GetDebugger()->HookCallback(pState, ld);
}

void LuaDebugger::HookCallback(lua_State* pState, lua_Debug *ld) {
	if (m_pState != pState) {
		//gLuaManager.TryError("检测到多个虚拟机");
		return;
	}

	int bFocus = false;
	switch (ld->event) {
	case LUA_HOOKCALL:


	case LUA_HOOKRET:


	case LUA_HOOKLINE:
		bFocus = true;
		break;
	}

	if (bFocus) {
		PrintContextLine();

		lua_sethook(m_pState, LuaDebugger::HooK, 0, 0);

		if (!m_isDebugging) {
			this->Debug();
		}
	}

}

int LuaDebugger::GetStackdepth() {
	int depth = 0;
	lua_Debug ld;
	while (lua_getstack(m_pState, depth, &ld)) {
		++depth;
	}

	return depth;
}

void LuaDebugger::PrintContextLine() {
	lua_Debug ar;
	int depth = GetStackdepth();
	for (int i = 0; i < depth; ++i) {

		lua_getstack(m_pState, i, &ar);
		lua_getinfo(m_pState, "S", &ar);
		lua_getinfo(m_pState, "l", &ar);
		if (ShowScriptLine(ar.source + 1 /*跳过 '@' 符号*/, ar.currentline)) {
			break;
		}
	}
}
