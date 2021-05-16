#pragma once
#ifndef SYMBOL_TABLE_H_INCLUDED
#define SYMBOL_TABLE_H_INCLUDED
#include <bits/stdc++.h>
#define rg_ne(i, a, b, s) for(auto i = a; i != b; i += s)
#define rg(i, a, b, s) for(auto i = a; (b > a)?(i < b):(i > b); i += s)
#define until(s) while(!(s))
using namespace std;
namespace symbol_table_space {

	enum KIND {
		VARIABLE, POINTER, FUNCTION, STRUCT_ID
	};
	string enum_to_string(KIND kind) {
		if (kind == KIND::POINTER) {
			return "POINTER";
		} else if (kind == KIND::FUNCTION) {
			return "FUNCTION";
		} else if (kind == KIND::STRUCT_ID) {
			return "STRUCT_ID";
		} else {
			return "VARIABLE";
		}
	}
	//unsigned int next_scope = 0;
	unordered_map<int, int> scopeLink;//作用域链，scopeLink[i] == j的意义为：j为i的父级作用域
	struct Symbol {
		//name为符号名。
		string name;
		//type为符号的类型。
		//单变量的值为其类型。
		//数组名的值为数组元素的类型。
		//函数的type为其返回值类型。
		//结构体标识符的值为struct-id。
		string type;
		//kind为符号的种类，与type的含义不同。
		//数组为ARRAY。
		//单变量为VARIABLE。
		//函数为FUNCTION。
		//结构体标识符为STRUCT_ID。
		KIND kind;
		//info记录了符号的一些信息，为以后的代码生成提供线索：
		//如果是普通变量，值为0。
		//如果是数组名，表示数组长度（元素个数）。特别地，如果是作为函数参数传入的数组名，则长度默认为INT_MAX。
		//如果是函数名，表示局部变量列表在符号表中的位置。
		//如果是结构体类型标识符，表示成员变量列表在符号表中的位置。
		int info;
		//updatable为1表示该符号可被修改（可作为左值出现在赋值语句中，如普通变量）
		//为0表示不可修改（如数组首地址，函数名）
		bool updatable;
		Symbol(string name, string type, KIND kind, int info, bool updatable = 1) {
			this->name = name;
			this->type = type;
			this->kind = kind;
			this->info = info;
			this->updatable = updatable;
		}
		string toString() {
			string _r = string("[name: ") + name;
			_r += string(", type: ") + type;
			_r += string(", kind: ") + enum_to_string(kind);
			_r += string(", info: ") + std::to_string(info) + string("]");
			return _r;
		}
	};
	//符号表
	vector<vector<Symbol*> > symbolTable;
	//函数参数表，记录各函数的参数个数
	unordered_map<string, int> parLen;
	//临时变量类型表，记录在函数内部产生的各临时变量的类型
	unordered_map<string, unordered_map<int, string> > tempType;
	int curScope = 0;
	void output(unordered_map<string, int> m) {
		for (auto it = m.begin(); it != m.end(); it++) {
			printf("%s: %d\n", it->first.c_str(), it->second);
		}
	}
	void addSymbol(string name, string type, KIND kind, int info, bool updatable = 1) {
		if (kind == KIND::VARIABLE && type == "void") {
			//尝试声明一个类型为void的变量会自动报错
			puts("error: invalid use of 'void'");
			exit(0);
		}
		if (kind == KIND::POINTER && info <= 0) {
			//尝试建立一个长度非正的数组会自动报错
			puts("error: size of array must be positive greater than 0.\a");
			printf("size: %d\n", info);
			exit(0);
		}
		symbolTable[curScope].push_back(new Symbol(name, type, kind, info, updatable));
	}
	//在整个符号表中寻找结构体标识符structId，此函数在目标代码生成阶段被调用
	pair<int, int> lookUpStructId(string structId) {
		for (int i = 0; i < symbolTable.size(); i++) {
			for (int j = 0; j < symbolTable[i].size(); j++) {
				auto cur = symbolTable[i][j];
				if (cur->kind == KIND::STRUCT_ID&&cur->name == structId)return make_pair(i, j);
			}
		}
		return make_pair(-1, 0);
	}
	//从当前作用域开始查找符号name，此函数在语义分析阶段被调用
	pair<int, int> lookup(string name) {
		int scope = curScope;
		while (scope != -1) {
			for (int i = 0; i < symbolTable[scope].size(); i++) {
				if (symbolTable[scope][i]->name == name) {
					//                printf("Scope: %d\n", father);
					return make_pair(scope, i);
				}
			}
			scope = scopeLink[scope];
		}
		return make_pair(-1, 0);
	}
	//在指定的作用域scope中寻找符号name，在目标代码生成阶段调用此函数
	pair<int, int> lookup(string name, int scope) {
		for (int i = 0; i < symbolTable[scope].size(); i++) {
			if (symbolTable[scope][i]->name == name)return make_pair(scope, i);
		}
		return make_pair(scope, -1);
	}
	string lookupType(int scope, string name) {
		for (Symbol* symbol: symbolTable[scope]) {
			if (symbol->name == name) {
				return symbol->type;
			}
		}
		return "";
	}
	void enterScope();
	void exitScope();
	void addSystemCalling() {
		/*
			void printInt(int i)
			在屏幕上输出一个整数
			i：要输出的整数
		*/
		addSymbol("printInti", "v", KIND::FUNCTION, 0);
		int *tobeUpdated = &symbolTable[0][0]->info;
		enterScope();
		addSymbol("i", "i", KIND::VARIABLE, 0);
		*tobeUpdated = curScope;
		exitScope();
		/*
			void printSpace()
			在屏幕上输出一个空格
		*/
		addSymbol("printSpacev", "v", KIND::FUNCTION, 0);
		tobeUpdated = &symbolTable[0][1]->info;
		enterScope();
		*tobeUpdated = curScope;
		exitScope();
		/*
			void printIntLn(int i)
			在屏幕上输出一个整数，然后输出一个换行符
			i：要输出的整数
		*/
		addSymbol("printIntLni", "v", KIND::FUNCTION, 0);
		tobeUpdated = &symbolTable[0][0]->info;
		enterScope();
		addSymbol("i", "i", KIND::VARIABLE, 0);
		*tobeUpdated = curScope;
		exitScope();
		/*
			void printLn()
			在屏幕上输出一个换行符
		*/
		addSymbol("printLnv", "v", KIND::FUNCTION, 0);
		tobeUpdated = &symbolTable[0][0]->info;
		enterScope();
		addSymbol("i", "i", KIND::VARIABLE, 0);
		*tobeUpdated = curScope;
		exitScope();
		/*
			void printChar(int i)
			在屏幕上输出一个字符
			i：要输出的字符的ascii编码值
		*/
		addSymbol("printChari", "v", KIND::FUNCTION, 0);
		tobeUpdated = &symbolTable[0][1]->info;
		enterScope();
		*tobeUpdated = curScope;
		exitScope();
	}
	void initiation() {
		curScope = 0;
		symbolTable.push_back(vector<Symbol*>());
		scopeLink[0] = -1;
		addSystemCalling();
	}

	void enterScope() {
		//puts("进入作用域");
		int tmp = curScope;
		curScope = symbolTable.size();
		scopeLink[curScope] = tmp;
		symbolTable.push_back(vector<Symbol*>());
	}
	void exitScope() {
		//puts("退出作用域");
		curScope = scopeLink[curScope];
	}
	void outputTable() {
		puts("symbol table:");
		int cnt = 0;
		for (auto v : symbolTable) {
			cnt++;
			for (auto e : v) {
				//            printf("%s ", e->name.c_str());
				cout << e->toString() << endl;
			}
			puts("");
		}
	}

}

#endif // SYMBOL_TABLE_H_INCLUDED
