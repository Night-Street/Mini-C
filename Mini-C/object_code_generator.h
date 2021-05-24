#pragma once
#include "quadruples.h"
#define MASMFunction MASMProcedure
namespace object_code_generator {
	auto curIt = threeAddrCodes.begin();
	int labelCnt = 0;
	string createNewLabel() {
		labelCnt++;
		return "L" + to_string(labelCnt);
	}
	void clearLabelCnt() {
		labelCnt = 0;
	}
	struct LocalInfo {
		string name;
		int scope;
		string type;
		static LocalInfo parseLocalInfo(string var) {
			LocalInfo li;
			string name;
			int scope;
			for (int i = 1; i < var.length(); i++) {
				if (!isdigit(var[i])) {
					int nameLen = stoi(var.substr(1, i - 1));
					name = var.substr(i, nameLen);
					scope = stoi(var.substr(i + nameLen));
					break;
				}
			}
			string type = lookupType(scope, name);
			li.name = name;
			li.scope = scope;
			li.type = type;
			return li;
		}
	};
	struct GlobalInfo {
		string name;
		string type;
		static GlobalInfo parseGlobalInfo(string var) {
			GlobalInfo gi;
			string name;
			for (int i = 1; i < var.length(); i++) {
				if (!isdigit(var[i])) {
					name = var.substr(i);
					break;
				}
			}
			string type = lookupType(0, name);
			gi.name = name;
			gi.type = type;
			return gi;
		}
	};
	struct TempInfo {
		string name;
		int idx;
		string type;
		static TempInfo parseTempInfo(string functionName, string var) {
			TempInfo ti;
			string name = var;
			int tmpIdx = stoi(var.substr(1));
			string type = tempType[functionName][tmpIdx];
			ti.name = name;
			ti.idx = tmpIdx;
			ti.type = type;
			return ti;
		}
	};
	struct ParameterInfo {
		int idx;
		string type;
		int offset;
		static ParameterInfo parseParameterInfo(string functionName, string var) {
			ParameterInfo pi;
			int idx = stoi(var.substr(1));
			auto functionPos = lookup(functionName, 0);
			auto subScope = symbolTable[functionPos.first][functionPos.second]->info;
			string type = symbolTable[subScope][idx - 1]->type;
			int offset;
			string returnType = symbolTable[functionPos.first][functionPos.second]->type;
			if (returnType == "i" || returnType == "v") {
				offset = idx * 4 + 4;
			} else {
				offset = idx * 4 + 8;
			}
			pi.idx = idx;
			pi.type = type;
			pi.offset = offset;
			return pi;
		}
	};
	vector<string> objectCode;
	class MASMProcedure {
	public:
		//目标代码表
		vector<string> objectCodeList;
		//局部变量声明语句表
		unordered_map<string, string> localDeclarationMap;
		//局部变量类型表
		unordered_map<string, string> localDeclarationType;
		//局部结构体声明语句表
		unordered_map<string, vector<string> > structDeclarationMap;
		//函数名
		string name;
		MASMProcedure(string name) {
			this->name = name;
		}
		//添加局部变量声明
		void appendLocalVar(string name, string type) {
			if (name[0] == 't') {
				if (type[0] == 'P') {
					//指针类型的临时变量，可以直接按照dword来处理
					//type = "i";
					localDeclarationType[name] = "i";
					localDeclarationMap[name] = "local " + name + ":dword";
				} else {
					if (type == "i") {
						localDeclarationType[name] = "i";
						localDeclarationMap[name] = "local " + name + ":dword";
					} else {
						localDeclarationType[name] = type;
						localDeclarationMap[name] = "local " + name + ":" + type;
					}
				}
			} else {
				//非临时变量
				if (type[0] == 'P') {
					int arrayLen;
					if (name[0] == 'L') {
						auto li = LocalInfo::parseLocalInfo(name);
						auto pos = lookup(li.name, li.scope);
						arrayLen = symbolTable[pos.first][pos.second]->info;
					}
					if (type[1] == 'i') {
						localDeclarationType[name] = "Pi";
						localDeclarationMap[name] = "local " + name + "[" + to_string(arrayLen) + "]:dword";
					} else {
						string dtype;
						for (int i = 1; i < type.length(); i++) {
							if (!isdigit(type[i])) {
								dtype = type.substr(i);
								break;
							}
						}
						localDeclarationType[name] = type;
						localDeclarationMap[name] = "local " + name + "[" + to_string(arrayLen) + "]:" + dtype;
					}
				} else {
					if (type == "i") {
						localDeclarationType[name] = "i";
						localDeclarationMap[name] = "local " + name + ":dword";
					} else {
						localDeclarationType[name] = type;
						localDeclarationMap[name] = "local " + name + ":" + type;
					}
				}
			}
		}
		//全局变量声明语句
		static unordered_map<string, string> globalMap;
		//全局变量类型表
		static unordered_map<string, string> globalType;
		//全局结构体标识符表
		static unordered_map<string, vector<string> > globalStructMap;
		//添加全局变量声明
		static void appendGlobalVar(string name, string type) {
			if (type[0] == 'P') {
				auto gi = GlobalInfo::parseGlobalInfo(name);
				auto pos = lookup(gi.name, 0);
				int arrayLen = symbolTable[pos.first][pos.second]->info;
				if (type[1] == 'i') {
					globalType[name] = "Pi";
					globalMap[name] = name + " dword " + to_string(arrayLen) + " dup (?)";
				} else {
					string dtype;
					for (int i = 1; i < type.length(); i++) {
						if (!isdigit(type[i])) {
							dtype = type.substr(i);
							break;
						}
					}
					globalType[name] = type;
					globalMap[name] = name + " " + dtype + " " + to_string(arrayLen) + " dup (<?>)";
				}
			} else {
				if (type == "i") {
					globalMap[name] = name + " dword ?";
					globalType[name] = "i";
				} else {
					globalMap[name] = name + " " + type + " ?";
					globalType[name] = type;
				}
			}
		}
	};
	unordered_map<string, vector<string> > MASMFunction::globalStructMap = unordered_map<string, vector<string> >();
	unordered_map<string, string> MASMFunction::globalMap = unordered_map<string, string>();
	unordered_map<string, string> MASMFunction::globalType = unordered_map<string, string>();
	unordered_map<string, MASMFunction* > functionMap;
	void initiation() {
		quadruple_space::initiation();
		clearLabelCnt();
	}
	string parseType(string curFunction,string var) {
		if (isdigit(var[0]))return "i";
		string ret;
		switch (var[0]) {
		case 'G': {
			ret= MASMFunction::globalType[var];
			break;
		}
		case 'P': {
			ret= ParameterInfo::parseParameterInfo(curFunction, var).type;
			break;

		}
		default: {
			ret = functionMap[curFunction]->localDeclarationType[var];
			break;
		}
		}
		return ret;
	}
	vector<string> translateStatement(string curFunction, Quadruples* q) {
		vector<string> ret;
		//[].xxx=
		if (q->op.length() > 4 && q->op.substr(0, 3) == "[]." && q->op.back()=='=') {
			if (q->result[0] == 'P') {
				auto pi = ParameterInfo::parseParameterInfo(curFunction, q->result);
				ret.push_back("mov edi, [ebp+" + to_string(pi.offset) + "]");
				auto pos = lookup(curFunction, 0);
				int subScope = symbolTable[pos.first][pos.second]->info;
				string type = symbolTable[subScope][pi.idx - 1]->type;
				for (int i = 1; i < type.length(); i++) {
					if (!isdigit(type[i])) {
						type = type.substr(i);
						break;
					}
				}
				pos = lookUpStructId(type);
				subScope = symbolTable[pos.first][pos.second]->info;
				string field = q->op.substr(3, q->op.length() - 4);
				ret.push_back("mov eax, " + q->opnd1);
				ret.push_back("imul eax, "+to_string(symbolTable[subScope].size()*4));
				ret.push_back("add eax, edi");
				ret.push_back("mov edx, " + q->opnd2);
				for (int i = 0; i < symbolTable[subScope].size(); i++) {
					if (symbolTable[subScope][i]->name == field) {
						ret.push_back("mov [eax+" + to_string(i * 4) + "], edx");
						break;
					}
				}
			} else {
				string field = q->op.substr(3, q->op.length() - 4);
				ret.push_back("lea edi, " + q->result);
				string type = parseType(curFunction,q->result);
				ret.push_back("mov eax, " + q->opnd1);
				for (int i = 1; i < type.length(); i++) {
					if (!isdigit(type[i])) {
						type = type.substr(i);
						break;
					}
				}
				auto pos = lookUpStructId(type);
				int subScope = symbolTable[pos.first][pos.second]->info;
				ret.push_back("imul eax, " + to_string(symbolTable[subScope].size() * 4));
				ret.push_back("add eax, edi");
				ret.push_back("mov edx, " + q->opnd2);
				for (int i = 0; i < symbolTable[subScope].size(); i++) {
					if (symbolTable[subScope][i]->name == field) {
						ret.push_back("mov [eax+" + to_string(i * 4) + "], edx");
						break;
					}
				}
			}
		}
		if (q->op == "par")ret.push_back("push " + q->result);
		if (q->op[0] == 'l')ret.push_back(q->op + ":");
		if (q->op == "jmp")ret.push_back(q->op + " " + q->result);
		if (q->op == "jz") {
			if (isdigit(q->opnd1[0])) {
				ret.push_back("mov eax, " + q->opnd1);
				ret.push_back("cmp eax, 0");
			} else {
				ret.push_back("cmp " + q->opnd1 + ", 0");
			}
			ret.push_back("je " + q->result);
		}
		if (q->op == "call") {
			string functionType = lookupType(0, q->opnd1);
			//先判断是不是系统调用
			if (q->opnd1 == "printChari") {
				ret.push_back("pop eax");
				ret.push_back("call dispc");
			}else if (q->opnd1 == "printLnv") {
				ret.push_back("call dispcrlf");
			}else if (q->opnd1 == "printIntLni") {
				ret.push_back("pop eax");
				ret.push_back("call dispsid");
				ret.push_back("call dispcrlf");
			} else if (q->opnd1 == "printSpacev") {
				ret.push_back("mov eax, 32");
				ret.push_back("call dispc");
			}else if(q->opnd1=="printInti"){
				ret.push_back("pop eax");
				ret.push_back("call dispsid");
			} else {
				if (functionType == "i") {
					ret.push_back("call " + q->opnd1);
					ret.push_back("mov " + q->result + ", eax");
				} else if (functionType == "v") {
					ret.push_back("call " + q->opnd1);
				} else {
					ret.push_back("push " + q->result);
					ret.push_back("call " + q->opnd1);
				}

			}
		}
		if (q->op == "ret") {
			string functionType = lookupType(0, curFunction);
			if (functionType == "i") {
				ret.push_back("mov eax, " + q->result);
				ret.push_back("ret " + to_string(parLen[curFunction] * 4));
			} else if (functionType == "v") {
				ret.push_back("ret " + to_string(parLen[curFunction] * 4));
			} else {
				auto pos = lookUpStructId(functionType);
				int subScope = symbolTable[pos.first][pos.second]->info;
				ret.push_back("mov edi, [ebp+8]");
				for (int i = 0; i < symbolTable[subScope].size(); i++) {
					Symbol*s = symbolTable[subScope][i];
					ret.push_back("mov eax, " + q->result + "." + s->name);
					ret.push_back("mov [edi+" + to_string(i * 4) + "], eax");
				}
				if (functionType != "i" && functionType != "v") {
					ret.push_back("ret " + to_string(parLen[curFunction] * 4 + 4));
				} else {
					ret.push_back("ret " + to_string(parLen[curFunction] * 4));
				}
			}
		}
		//.xxx=
		if (q->op[0] == '.' && q->op.back() == '=') {
			if (q->result[0] == 'P') {
				ParameterInfo pi = ParameterInfo::parseParameterInfo(curFunction, q->result);
				auto pos = lookUpStructId(pi.type);
				string field = q->op.substr(1, q->op.length() - 2);
				int subScope = symbolTable[pos.first][pos.second]->info;
				int offset;
				for (int i = 0; i < symbolTable[subScope].size(); i++) {
					if (symbolTable[subScope][i]->name == field) {
						offset = 4 * i;
						break;
					}
				}
				ret.push_back("mov edi, [ebp+" + to_string(pi.offset) + "]");
				ret.push_back("mov eax, " + q->opnd1);
				ret.push_back("mov [edi+" + to_string(offset) + "], eax");
			} else {
				string field = q->op.substr(1, q->op.length() - 2);
				ret.push_back("mov eax, " + q->opnd1);
				ret.push_back("mov " + q->result + "." + field + ", eax");
			}
		}
		//=.xxx
		if (q->op.length() > 2 && q->op[0] == '=' && q->op[1] == '.') {
			if (q->opnd1[0] == 'P') {
				ParameterInfo pi = ParameterInfo::parseParameterInfo(curFunction, q->opnd1);
				auto pos = lookUpStructId(pi.type);
				string field = q->op.substr(2);
				int subScope = symbolTable[pos.first][pos.second]->info;
				int offset;
				for (int i = 0; i < symbolTable[subScope].size(); i++) {
					if (symbolTable[subScope][i]->name == field) {
						offset = 4 * i;
						break;
					}
				}
				ret.push_back("mov esi, [ebp+" + to_string(pi.offset) + "]");
				ret.push_back("mov eax, [esi+" + to_string(offset) + "]");
				ret.push_back("mov " + q->result + ", eax");
			} else {
				string field = q->op.substr(2);
				ret.push_back("mov eax, " + q->opnd1 + "." + field);
				ret.push_back("mov " + q->result + ", eax");
			}
		}
		if (q->op == "==") {
			string inner = createNewLabel();
			string outer = createNewLabel();
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("cmp eax, " + q->opnd2);
			ret.push_back("je " + inner);
			ret.push_back("mov " + q->result + ", 0");
			ret.push_back("jmp " + outer);
			ret.push_back(inner + ":");
			ret.push_back("mov " + q->result + ", 1");
			ret.push_back(outer + ":");
		}
		if (q->op == "!=") {
			string inner = createNewLabel();
			string outer = createNewLabel();
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("cmp eax, " + q->opnd2);
			ret.push_back("jne " + inner);
			ret.push_back("mov " + q->result + ", 0");
			ret.push_back("jmp " + outer);
			ret.push_back(inner + ":");
			ret.push_back("mov " + q->result + ", 1");
			ret.push_back(outer + ":");
		}
		if (q->op == "<") {
			string inner = createNewLabel();
			string outer = createNewLabel();
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("cmp eax, " + q->opnd2);
			ret.push_back("js " + inner);
			ret.push_back("mov " + q->result + ", 0");
			ret.push_back("jmp " + outer);
			ret.push_back(inner + ":");
			ret.push_back("mov " + q->result + ", 1");
			ret.push_back(outer + ":");
		}
		if (q->op == "<=") {
			string inner = createNewLabel();
			string outer = createNewLabel();
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("cmp eax, " + q->opnd2);
			ret.push_back("js " + inner);
			ret.push_back("je " + inner);
			ret.push_back("mov " + q->result + ", 0");
			ret.push_back("jmp " + outer);
			ret.push_back(inner + ":");
			ret.push_back("mov " + q->result + ", 1");
			ret.push_back(outer + ":");
		}
		if (q->op == ">") {
			string inner = createNewLabel();
			string outer = createNewLabel();
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("cmp eax, " + q->opnd2);
			ret.push_back("jns " + inner);
			ret.push_back("mov " + q->result + ", 0");
			ret.push_back("jmp " + outer);
			ret.push_back(inner + ":");
			ret.push_back("mov " + q->result + ", 0");
			ret.push_back("je " + outer);
			ret.push_back("mov " + q->result + ", 1");
			ret.push_back(outer + ":");
		}
		if (q->op == ">=") {
			string inner = createNewLabel();
			string outer = createNewLabel();
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("cmp eax, " + q->opnd2);
			ret.push_back("jns " + inner);
			ret.push_back("mov " + q->result + ", 0");
			ret.push_back("jmp " + outer);
			ret.push_back(inner + ":");
			ret.push_back("mov " + q->result + ", 1");
			ret.push_back(outer + ":");
		}
		if (q->op == "[]=") {
			string type = parseType(curFunction, q->opnd2);
			if (type == "i") {
				if (q->result[0] == 'P') {
					auto pi = ParameterInfo::parseParameterInfo(curFunction, q->result);
					ret.push_back("mov edi, [ebp+" + to_string(pi.offset) + "]");
				} else if (q->result[0] == 't') {
					ret.push_back("mov edi, " + q->result);
				} else {
					ret.push_back("lea edi, " + q->result);
				}
				ret.push_back("mov eax, " + q->opnd1);
				ret.push_back("imul eax, 4");
				ret.push_back("add eax, edi");
				ret.push_back("mov edx, " + q->opnd2);
				ret.push_back("mov [eax], edx");
			} else {
				auto pos = lookUpStructId(type);
				int subScope = symbolTable[pos.first][pos.second]->info;
				if (q->result[0] == 'P') {
					auto pi = ParameterInfo::parseParameterInfo(curFunction, q->result);
					ret.push_back("mov edi, [ebp+" + to_string(pi.offset) + "]");
				} else if (q->result[0] == 't') {
					ret.push_back("mov edi, " + q->result);
				} else {
					ret.push_back("lea edi, " + q->result);
				}
				ret.push_back("mov eax, " + q->opnd1);
				ret.push_back("imul eax, " + to_string(symbolTable[subScope].size()*4));
				ret.push_back("add eax, edi");
				for (int i = 0; i < symbolTable[subScope].size(); i++) {
					ret.push_back("mov edx, " + q->opnd2 + "." + symbolTable[subScope][i]->name);
					ret.push_back("mov [eax+" + to_string(i * 4) + "], edx");
				}

			}
		}
		if (q->op == "=[]") {
			string type = parseType(curFunction, q->result);
			if (type == "i") {
				if (q->opnd1[0] == 'P') {
					auto pi = ParameterInfo::parseParameterInfo(curFunction, q->opnd1);
					ret.push_back("mov esi, [ebp+" + to_string(pi.offset) + "]");
				} else {
					ret.push_back("lea esi, " + q->opnd1);
				}
				ret.push_back("mov eax, " + q->opnd2);
				ret.push_back("imul eax, 4");
				ret.push_back("add eax, esi");
				ret.push_back("mov edx, [eax]");
				ret.push_back("mov " + q->result + ", edx");
			} else {
				auto pos = lookUpStructId(type);
				int subScope = symbolTable[pos.first][pos.second]->info;
				if (q->opnd1[0] == 'P') {
					auto pi = ParameterInfo::parseParameterInfo(curFunction, q->opnd1);
					ret.push_back("mov esi, [ebp+" + to_string(pi.offset) + "]");
				} else {
					ret.push_back("lea esi, " + q->opnd1);
				}
				ret.push_back("mov eax, " + q->opnd2);
				ret.push_back("imul eax, " + to_string(symbolTable[subScope].size()*4));
				ret.push_back("add eax, esi");
				for (int i = 0; i < symbolTable[subScope].size(); i++) {
					ret.push_back("mov edx, [eax+" + to_string(i * 4) + "]");
					ret.push_back("mov " + q->result + "." + symbolTable[subScope][i]->name + ", edx");
				}
			}
		}
		if (q->op == "=") {
			string left_type = "", right_type = "";
			left_type = parseType(curFunction, q->result);
			right_type = parseType(curFunction, q->opnd1);
			if (left_type == "i") {
				if (right_type == "i") {
					if (q->result[0] == 'P') {
						auto pi = ParameterInfo::parseParameterInfo(curFunction, q->result);
						ret.push_back("mov eax, " + q->opnd1);
						ret.push_back("mov [ebp+" + to_string(pi.offset) + "], eax");
					} else if (q->opnd1[0] == 'P') {
						auto pi = ParameterInfo::parseParameterInfo(curFunction, q->opnd1);
						ret.push_back("mov eax, [ebp+" + to_string(pi.offset) + "]");
						ret.push_back("mov " + q->result + ", eax");
					} else {
						ret.push_back("mov eax, " + q->opnd1);
						ret.push_back("mov " + q->result + ", eax");
					}
				} else {
					if (q->result[0] == 'P') {
						ret.push_back("lea eax, " + q->opnd1);
						auto pi = ParameterInfo::parseParameterInfo(curFunction, q->result);
						ret.push_back("mov [ebp+" + to_string(pi.offset) + "], eax");
					} else if (q->opnd1[0] == 'P') {
						auto pi = ParameterInfo::parseParameterInfo(curFunction, q->opnd1);
						ret.push_back("mov eax, [ebp+" + to_string(pi.offset) + "]");
						ret.push_back("mov " + q->result + ", eax");
					} else {
						ret.push_back("lea eax, " + q->opnd1);
						ret.push_back("mov " + q->result + ", eax");
					}
				}
			} else if (left_type.length() >= 2 && left_type[0] == 'P' && isdigit(left_type[1]) || left_type == "Pi") {
				//左值类型为指针类型
				ret.push_back("mov eax, " + q->opnd1);
				if (q->result[0] == 'P') {
					auto pi = ParameterInfo::parseParameterInfo(curFunction, q->result);
					ret.push_back("mov [ebp+" + to_string(pi.offset) + "], eax");
				} else {
					cerr << "error";
					exit(0);
				}
			} else {
				//结构体变量的赋值
				auto pos = lookUpStructId(left_type);
				/*if (pos.first == -1) {
					cerr << flag << " -1" << endl;
					cout << q->result;
					exit(0);
				}*/
				auto structId = symbolTable[pos.first][pos.second];
				int subScope = structId->info;
				if (q->opnd1[0] == 'P') {
					//如果右值是函数参数
					ParameterInfo pi = ParameterInfo::parseParameterInfo(curFunction, q->opnd1);
					//参数值送esi
					ret.push_back("mov esi, [ebp+" + to_string(pi.offset) + "]");
					for (int i = 0; i < symbolTable[subScope].size(); i++) {
						auto curSymbol = symbolTable[subScope][i];
						//作指针运算，将结构体成员变量依次赋值，通过eax完成
						ret.push_back("mov eax, [esi+" + to_string(i * 4) + "]");
						ret.push_back("mov " + q->result + "." + curSymbol->name + ", eax");
					}
				} else if (q->result[0] == 'P') {
					//如果左值是函数参数
					ParameterInfo pi = ParameterInfo::parseParameterInfo(curFunction, q->result);
					//参数值送edi
					ret.push_back("mov edi, [ebp+" + to_string(pi.offset) + "]");
					for (int i = 0; i < symbolTable[subScope].size(); i++) {
						auto curSymbol = symbolTable[subScope][i];
						//作指针运算，将结构体成员变量依次赋值，通过eax完成
						ret.push_back("mov eax, " + q->opnd1 + "." + curSymbol->name);
						ret.push_back("mov [edi+" + to_string(i * 4) + "], eax");
					}
				} else {
					//否则将各个成员变量依次赋值，通过eax完成
					for (auto s : symbolTable[subScope]) {
						ret.push_back("mov eax, " + q->opnd1 + "." + s->name);
						ret.push_back("mov " + q->result + "." + s->name + ", eax");
					}

				}
			}
		}
		if (q->op == "+") {
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("add eax, " + q->opnd2);
			ret.push_back("mov " + q->result + ", eax");
		}
		if (q->op == "-") {
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("sub eax, " + q->opnd2);
			ret.push_back("mov " + q->result + ", eax");
		}
		if (q->op == "*") {
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("imul eax, " + q->opnd2);
			ret.push_back("mov " + q->result + ", eax");
		}
		if (q->op == "/") {
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("mov edx, 0");
			ret.push_back("mov ecx, " + q->opnd2);
			ret.push_back("idiv ecx");
			ret.push_back("mov " + q->result + ", eax");
		}
		if (q->op == "%") {
			ret.push_back("mov eax, " + q->opnd1);
			ret.push_back("mov edx, 0");
			ret.push_back("mov ecx, " + q->opnd2);
			ret.push_back("idiv ecx");
			ret.push_back("mov " + q->result + ", edx");
		}
		return ret;
	}
	void addVariableDeclaration(MASMFunction* fb, string& var) {
		switch (var[0]) {
			//局部变量
		case 'L': {
			LocalInfo li = LocalInfo::parseLocalInfo(var);
			string name = li.name;
			int scope = li.scope;
			string type = li.type;
			fb->appendLocalVar(var, type);
			break;
		}
				  //全局变量
		case 'G': {
			GlobalInfo gi = GlobalInfo::parseGlobalInfo(var);
			string name = gi.name;
			string type = gi.type;
			MASMFunction::appendGlobalVar(var, type);
			break;
		}
				  //临时变量
		case 't': {
			TempInfo ti = TempInfo::parseTempInfo(fb->name, var);
			string name = ti.name;
			int tmpIdx = ti.idx;
			string type = ti.type;
			fb->appendLocalVar(name, type);
			break;
		}
		}
	}
	vector<string> translateFunction(MASMFunction* fb, Quadruples* q) {
		if (q->op == "struct") {
			//声明局部结构体标识符
			string structName = q->result;
			fb->structDeclarationMap[structName].clear();
			fb->structDeclarationMap[structName].push_back(structName + " struct");
			while (1) {
				auto curInstr = getNextQuadruple();
				if (curInstr->op == "ends") {
					fb->structDeclarationMap[structName].push_back(structName + " ends");
					break;
				}
				fb->structDeclarationMap[structName].push_back(q->result + " dword ?");
			}
			return vector<string>();
		}
		addVariableDeclaration(fb, q->opnd1);
		addVariableDeclaration(fb, q->opnd2);
		addVariableDeclaration(fb, q->result);
		vector<string> ret = translateStatement(fb->name, q);
		return ret;
	}
	//第一遍扫描，将所有的中间代码语句翻译成目标代码语句，并添加全局变量、局部变量、临时变量和结构体标识符的声明
	void firstPass() {
		//for (auto x : tempType) {
		//	puts(x.first.c_str());
		//	for (auto p : x.second) {
		//		printf("%d:%s\n", p.first, p.second.c_str());
		//	}
		//	puts("");
		//}
		//return;
		string curFunctionName = "";
		while (1) {
			//cout << curFunctionName << endl;
			auto curInstr = getNextQuadruple();
			if (!curInstr)break;
			//子程序声明
			if (curInstr->op == "proc") {
				curFunctionName = curInstr->result;
				auto curFunction = new MASMFunction(curFunctionName);
				curFunction->objectCodeList.push_back(curFunctionName + " proc");
				functionMap[curFunctionName] = curFunction;
				continue;
			}
			//结束子程序声明
			if (curInstr->op == "endp") {
				clearLabelCnt();

				auto curFunction = functionMap[curFunctionName];
				//无论函数有没有return语句，在函数末尾自动生成ret指令
				string functionType = lookupType(0, curFunctionName);
				if (functionType != "i" && functionType != "v") {
					curFunction->objectCodeList.push_back("ret " + to_string(parLen[curFunctionName] * 4 + 4));
				} else {
					curFunction->objectCodeList.push_back("ret " + to_string(parLen[curFunctionName] * 4));
				}
				//curFunction->objectCodeList.push_back("ret " + to_string((parLen[curFunction->name] + (functionType == "i")) * 4));
				curFunction->objectCodeList.push_back(curFunctionName + " endp");
				
				curFunctionName = "";
				continue;
			}
			//全局struct声明
			if (curInstr->op == "struct") {
				string structName = curInstr->result;
				MASMFunction::globalStructMap[structName].clear();
				MASMFunction::globalStructMap[structName].push_back(structName + " struct");
				while (1) {
					curInstr = getNextQuadruple();
					if (curInstr->op == "ends") {
						MASMFunction::globalStructMap[structName].push_back(structName + " ends");
						break;
					}
					MASMFunction::globalStructMap[structName].push_back(curInstr->result + " dword ?");
				}
				continue;
			}
			for (auto s : translateFunction(functionMap[curFunctionName], curInstr))functionMap[curFunctionName]->objectCodeList.push_back(s);

		}
	}
	//第二遍扫描，对于没有局部变量或临时变量的函数，手动将ebp压入栈并设置ebp的新值，并在函数结尾处将ebp弹出栈
	void secondPass() {
		vector<string> objectCodeList;
		//freopen("object code.txt", "r", stdin);
		string lastLine, curLine;
		lastLine = objectCode.front();
		//if (lastLine.back() == '\r')lastLine.pop_back();
		objectCodeList.push_back(lastLine);
		bool flag = 0;
		for (auto it = next(objectCode.begin()); it != objectCode.end(); it++) {
			curLine = *it;
			//if (curLine.back() == '\r')curLine.pop_back();
			if (!flag
				&& lastLine.length() > 6
				&& lastLine.substr(lastLine.length() - 4) == "proc"
				&& curLine.length() >= 5
				&& curLine.substr(0, 5) != "local") {
				flag = 1;
				objectCodeList.push_back("push ebp");
				objectCodeList.push_back("mov ebp, esp");
			}
			if (flag
				&& curLine.length() >= 3
				&& curLine.substr(0, 3) == "ret") {
				objectCodeList.push_back("pop ebp");
			}
			if (flag
				&& curLine.length() >= 4
				&& curLine.substr(curLine.length() - 4) == "endp") {
				flag = 0;
			}
			objectCodeList.push_back(curLine);
			lastLine = curLine;
		}
		swap(objectCode, objectCodeList);
	}
	//第三遍扫描，对于结构体类型和指针类型变量，压栈时添加一条lea指令，把地址传给eax，然后将eax压入栈
	void thirdPass() {
		vector<string> objectCodeList;
		string functionName;
		for (string s : objectCode) {
			if (s.length() >= 4
				&& s.substr(s.length() - 4) == "proc") {
				functionName = s.substr(0, s.length() - 5);
			}
			vector<string> cache;
			if (s.length() >= 4
				&& s.substr(0, 4) == "push") {
				string varName = s.substr(5);
				if (varName[0] == 'G'&&GlobalInfo::parseGlobalInfo(varName).type != "i") {
					cache.push_back("lea eax, " + varName);
					s = "push eax";
				} else if (varName[0] == 'L'&&LocalInfo::parseLocalInfo(varName).type != "i") {
					cache.push_back("lea eax, " + varName);
					s = "push eax";
				} else if (varName[0] == 't') {
					auto type = TempInfo::parseTempInfo(functionName, varName).type;
					if (type != "i"&&type[0] != 'P') {
						cache.push_back("lea eax, " + varName);
						s = "push eax";
					}
				}
			}
			for (string s : cache)objectCodeList.push_back(s);
			objectCodeList.push_back(s);
		}
		swap(objectCode, objectCodeList);
	}
	//将第一遍扫描结果储存在objectCode中
	void saveObjectCode() {
		objectCode.push_back("include io32.inc");
		objectCode.push_back(".data");
		for (auto p : MASMFunction::globalStructMap) {
			for (string instr : p.second) {
				objectCode.push_back(instr);
			}
		}
		for (auto p : MASMFunction::globalMap) {
			objectCode.push_back(p.second);
		}
		//for (auto p : functionMap["mainv"]->localDeclarationMap)fprintf(fout, "\t%s\n", p.second.c_str());
		
		objectCode.push_back(".code");
		objectCode.push_back("start:");
		objectCode.push_back("call mainv");
		objectCode.push_back("exit 0");

		//输出子程序
		for (auto p : functionMap) {
			//输出子程序的局部变量和临时变量声明语句
			auto f = p.second;
			objectCode.push_back(f->objectCodeList.front());
			//fprintf(fout, "%s\n", f->objectCodeList.front().c_str());
			for (auto p : f->structDeclarationMap) {//输出子程序的局部结构体标识符定义
				for (string instr : p.second) {
					objectCode.push_back(instr);
				}
			}
			for (auto p : f->localDeclarationMap) {//输出子程序的局部变量和临时变量声明
				objectCode.push_back(p.second);
			}
			//输出子程序的语句代码
			for (auto it = next(f->objectCodeList.begin()); it != f->objectCodeList.end(); it++) {
				objectCode.push_back(*it);
			}
			delete f;//释放内存
		}
		functionMap.clear();//清空函数列表中的野指针
		objectCode.push_back("end start");
		//fprintf(fout, "end start\n");
	}
	//输出目标代码
	void printObjectCode(const char* outputFileName) {
		ofstream out;
		out.open(outputFileName);
		for (string s : objectCode) {
			bool f = 1;
			if (s[0] == '.')f = 0;
			else if (s.length() >= 4 && s.substr(s.length() - 4, 4) == "proc")f = 0;
			else if (s.length() >= 4 && s.substr(s.length() - 4, 4) == "endp")f = 0;
			if (f)out << '\t';
			out << s << endl;
		}
		out.close();
	}
	void endCodeGenerator() {
		tempType.clear();
		symbolTable.clear();
		MASMFunction::globalMap.clear();
		MASMFunction::globalStructMap.clear();
		MASMFunction::globalType.clear();
		functionMap.clear();
		objectCode.clear();
	}
	void updateFileExtension(string &si, string extension) {
		for (int i = si.length(); i != -1; i--) {
			if (si[i] == '.') {
				si.erase(i + 1);
				si += extension;
				break;
			}
		}
	}
	string objectCodeGenerator(const char* SI) {
		initiation();
		firstPass();
		saveObjectCode();
		initiation();
		secondPass();
		initiation();
		thirdPass();
		string si = SI;
		//把文件扩展名改为.asm
		updateFileExtension(si, "asm");
		printObjectCode(si.c_str());
		endCodeGenerator();
		return si;



	}
}