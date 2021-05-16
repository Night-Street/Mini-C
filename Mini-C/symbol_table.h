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
	unordered_map<int, int> scopeLink;//����������scopeLink[i] == j������Ϊ��jΪi�ĸ���������
	struct Symbol {
		//nameΪ��������
		string name;
		//typeΪ���ŵ����͡�
		//��������ֵΪ�����͡�
		//��������ֵΪ����Ԫ�ص����͡�
		//������typeΪ�䷵��ֵ���͡�
		//�ṹ���ʶ����ֵΪstruct-id��
		string type;
		//kindΪ���ŵ����࣬��type�ĺ��岻ͬ��
		//����ΪARRAY��
		//������ΪVARIABLE��
		//����ΪFUNCTION��
		//�ṹ���ʶ��ΪSTRUCT_ID��
		KIND kind;
		//info��¼�˷��ŵ�һЩ��Ϣ��Ϊ�Ժ�Ĵ��������ṩ������
		//�������ͨ������ֵΪ0��
		//���������������ʾ���鳤�ȣ�Ԫ�ظ��������ر�أ��������Ϊ����������������������򳤶�Ĭ��ΪINT_MAX��
		//����Ǻ���������ʾ�ֲ������б��ڷ��ű��е�λ�á�
		//����ǽṹ�����ͱ�ʶ������ʾ��Ա�����б��ڷ��ű��е�λ�á�
		int info;
		//updatableΪ1��ʾ�÷��ſɱ��޸ģ�����Ϊ��ֵ�����ڸ�ֵ����У�����ͨ������
		//Ϊ0��ʾ�����޸ģ��������׵�ַ����������
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
	//���ű�
	vector<vector<Symbol*> > symbolTable;
	//������������¼�������Ĳ�������
	unordered_map<string, int> parLen;
	//��ʱ�������ͱ���¼�ں����ڲ������ĸ���ʱ����������
	unordered_map<string, unordered_map<int, string> > tempType;
	int curScope = 0;
	void output(unordered_map<string, int> m) {
		for (auto it = m.begin(); it != m.end(); it++) {
			printf("%s: %d\n", it->first.c_str(), it->second);
		}
	}
	void addSymbol(string name, string type, KIND kind, int info, bool updatable = 1) {
		if (kind == KIND::VARIABLE && type == "void") {
			//��������һ������Ϊvoid�ı������Զ�����
			puts("error: invalid use of 'void'");
			exit(0);
		}
		if (kind == KIND::POINTER && info <= 0) {
			//���Խ���һ�����ȷ�����������Զ�����
			puts("error: size of array must be positive greater than 0.\a");
			printf("size: %d\n", info);
			exit(0);
		}
		symbolTable[curScope].push_back(new Symbol(name, type, kind, info, updatable));
	}
	//���������ű���Ѱ�ҽṹ���ʶ��structId���˺�����Ŀ��������ɽ׶α�����
	pair<int, int> lookUpStructId(string structId) {
		for (int i = 0; i < symbolTable.size(); i++) {
			for (int j = 0; j < symbolTable[i].size(); j++) {
				auto cur = symbolTable[i][j];
				if (cur->kind == KIND::STRUCT_ID&&cur->name == structId)return make_pair(i, j);
			}
		}
		return make_pair(-1, 0);
	}
	//�ӵ�ǰ������ʼ���ҷ���name���˺�������������׶α�����
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
	//��ָ����������scope��Ѱ�ҷ���name����Ŀ��������ɽ׶ε��ô˺���
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
			����Ļ�����һ������
			i��Ҫ���������
		*/
		addSymbol("printInti", "v", KIND::FUNCTION, 0);
		int *tobeUpdated = &symbolTable[0][0]->info;
		enterScope();
		addSymbol("i", "i", KIND::VARIABLE, 0);
		*tobeUpdated = curScope;
		exitScope();
		/*
			void printSpace()
			����Ļ�����һ���ո�
		*/
		addSymbol("printSpacev", "v", KIND::FUNCTION, 0);
		tobeUpdated = &symbolTable[0][1]->info;
		enterScope();
		*tobeUpdated = curScope;
		exitScope();
		/*
			void printIntLn(int i)
			����Ļ�����һ��������Ȼ�����һ�����з�
			i��Ҫ���������
		*/
		addSymbol("printIntLni", "v", KIND::FUNCTION, 0);
		tobeUpdated = &symbolTable[0][0]->info;
		enterScope();
		addSymbol("i", "i", KIND::VARIABLE, 0);
		*tobeUpdated = curScope;
		exitScope();
		/*
			void printLn()
			����Ļ�����һ�����з�
		*/
		addSymbol("printLnv", "v", KIND::FUNCTION, 0);
		tobeUpdated = &symbolTable[0][0]->info;
		enterScope();
		addSymbol("i", "i", KIND::VARIABLE, 0);
		*tobeUpdated = curScope;
		exitScope();
		/*
			void printChar(int i)
			����Ļ�����һ���ַ�
			i��Ҫ������ַ���ascii����ֵ
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
		//puts("����������");
		int tmp = curScope;
		curScope = symbolTable.size();
		scopeLink[curScope] = tmp;
		symbolTable.push_back(vector<Symbol*>());
	}
	void exitScope() {
		//puts("�˳�������");
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
