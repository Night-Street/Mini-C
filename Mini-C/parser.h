#pragma once
#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED
#include "scanner.h"
#include "symbol_table.h"
#include "quadruples.h"
//#include <bits/stdc++.h>
using namespace std;
using namespace symbol_table_space;
using namespace quadruple_space;
namespace parser {

	
	class SignList {
		string *sign;
		SignList *nxt;
	public:
		SignList(string *str) {
			sign = str;
			nxt = 0;
		}
		static void backPatch(SignList *cur, string str) {
			if (cur) {
				*cur->sign = str;
				backPatch(cur->nxt, str);
			}
		}
		static void suspend(SignList *&cur, string *str) {
			if (!cur)cur = new SignList(str);
			else suspend(cur->nxt, str);
		}
		static void delList(SignList *&cur) {
			if (!cur)return;
			delList(cur->nxt);
			delete cur;
			cur = 0;
		}
		static SignList* mergeList(SignList *l1, SignList *l2) {
			if (!l1)return l2;
			if (!l2)return l1;
			auto cur = l1;
			while (cur->nxt) {
				cur = cur->nxt;
			}
			cur->nxt = l2;
			return l1;
		}
	};
	
	struct Sign {
		string content;
		bool is_terminal;
		vector<Sign*> child;
		unordered_map<string, string> attributes;
		vector<Quadruples*> code;
		SignList *exit;
		SignList *breakList;
		SignList *continueList;
		Sign(string c) {
			this->content = c;
			this->is_terminal = 0;
			this->exit = 0;
			this->breakList = this->continueList = 0;
		}
	};
	bool jumpable = 0;
	bool continuable = 1;
	FILE* lexical_reader;
	//FILE* intermediate_code_generator;
	string cur_sign = "";
	queue<string> attributer_reader = scanner::synthesized_attribute;
	string cur_function = "";
	int tempCnt = 0;
	int labelCnt = 0;
	string createLabel() {
		labelCnt++;;
		return "l" + to_string(labelCnt);
	}
	string getLabel() {
		return "l" + to_string(labelCnt);
	}
	string createTemp() {
		tempCnt++;
		return "t" + to_string(tempCnt);
	}
	string getTemp() {
		return "t" + to_string(tempCnt);
	}
	void clearControlVar() {
		tempCnt = 0;
		labelCnt = 0;
	}
	void match(string expected_sign) {
		if (cur_sign == expected_sign) {
			//printf("Match successful: %s\n", cur_sign.c_str());
			char buffer[128];
			fscanf(lexical_reader, "%s", buffer);
			cur_sign = string(buffer);
		} else {
			puts("Syntax error!\a");
			printf("cur_symbol: %s\nMissing of expected_symbol: %s\n", cur_sign.c_str(), expected_sign.c_str());
			exit(0);
		}
	}
	string sign_advanced(int n) {
		stack<int> stk;
		char buffer[128];
		while (n--) {
			fscanf(lexical_reader, "%s", buffer);
			stk.push(strlen(buffer));
		}
		while (!stk.empty()) {
			fseek(lexical_reader, -(int)stk.top() - 1, SEEK_CUR);
			stk.pop();
		}
		return string(buffer);
	}
	string get_next_sign() {
		char buffer[128];
		fscanf(lexical_reader, "%s", buffer);
		fseek(lexical_reader, -(int)strlen(buffer) - 1, SEEK_CUR);
		return string(buffer);
	}
	unordered_multimap<string, vector<string> > productions;
	unordered_map<string, Sign*> sign_map;
	vector<string> par_types;
	unordered_map<string, unordered_set<string> > first, follow;
	//计算多个文法符号按顺序构成的first集合
	unordered_set<string> merge_first(vector<string> v) {
		unordered_set<string> s;
		rg(i, 0, v.size(), 1) {
			for (string _s : first[v[i]]) {
				if (_s != "") {
					s.insert(_s);
				}
			}
			if (first[v[i]].find("") == first[v[i]].end()) {
				break;
			}
			if (i == v.size() - 1) {
				s.insert("");
			}
		}
		return s;
	}
	unordered_set<string> merge_first(vector<string> v, int st, int ed) {
		unordered_set<string> s;
		rg(i, st, ed + 1, 1) {
			for (string _s : first[v[i]]) {
				if (_s != "") {
					s.insert(_s);
				}
			}
			if (first[v[i]].find("") == first[v[i]].end()) {
				break;
			}
			if (i == ed) {
				s.insert("");
			}
		}
		return s;
	}
	//通过词法分析器生成属性文件，获取当前终结符的综合属性
	string getCurAttr() {
		return attributer_reader.front();
		//char buffer[128];
		//fscanf(attribute_reader, "%s", buffer);
		//fseek(attribute_reader, -(int)strlen(buffer), SEEK_CUR);
		//return buffer;
	}
	string get_entry(pair<int, int> pos) {
		if (pos.first == 0) {
			//全局变量
			string name = symbolTable[pos.first][pos.second]->name;
			return "G" + to_string(name.length()) + name;
		} else {
			int ans = pos.second + 1;
			auto function_pos = lookup(cur_function);
			string function_name = symbolTable[function_pos.first][function_pos.second]->name;
			int sub_scope = symbolTable[function_pos.first][function_pos.second]->info;
			if (pos.first == sub_scope) {
				if (pos.second + 1 > parLen[function_name]) {
					string name = symbolTable[pos.first][pos.second]->name;
					return "L" + to_string(name.length()) + name + to_string(pos.first);
					//ans -= parLen[function_name];
					//return "local" + to_string(ans);
				} else {
					return "P" + to_string(ans);
				}
			} else {
				/*pos.first = scopeLink[pos.first];
				while (1) {
					if (pos.first == sub_scope) {
						ans += symbolTable[pos.first].size() - parLen[function_name];
						break;
					} else {
						ans += symbolTable[pos.first].size();
					}
					pos.first = scopeLink[pos.first];
				}*/
				string name = symbolTable[pos.first][pos.second]->name;
				return "L" + to_string(name.length()) + name + to_string(pos.first);
				//return "local" + to_string(ans);
			}
		}
	}
	//把函数的参数类型附加到函数名后面
	void attachTypes(string& id) {
		if (par_types.empty()) {
			//如果函数没有参数，则在符号表中的命名方式为IDv
			/*
				声明函数时，当获取了ID后，在符号表中暂时存储名为ID的函数声明。
				识别完所有参数后，根据ID找到此函数在符号表中的位置，根据参数类型在函数名后面按参数声明顺序加上参数类型后缀。
			*/
			id.push_back('v');
		} else {
			for (string cur_type : par_types) {
				id += cur_type;
			}
			par_types.clear();
		}
	}
	void set_terminals() {
		sign_map["int"]->is_terminal = 1;
		sign_map["void"]->is_terminal = 1;
		sign_map["struct"]->is_terminal = 1;
		sign_map["ID"]->is_terminal = 1;
		sign_map["["]->is_terminal = 1;
		sign_map["]"]->is_terminal = 1;
		sign_map["."]->is_terminal = 1;
		sign_map["{"]->is_terminal = 1;
		sign_map["}"]->is_terminal = 1;
		sign_map["NUM"]->is_terminal = 1;
		sign_map[","]->is_terminal = 1;
		sign_map[";"]->is_terminal = 1;
		sign_map["("]->is_terminal = 1;
		sign_map[")"]->is_terminal = 1;
		sign_map["if"]->is_terminal = 1;
		sign_map["else"]->is_terminal = 1;
		sign_map["switch"]->is_terminal = 1;
		sign_map["while"]->is_terminal = 1;
		sign_map["for"]->is_terminal = 1;
		sign_map["return"]->is_terminal = 1;
		sign_map["break"]->is_terminal = 1;
		sign_map["continue"]->is_terminal = 1;
		sign_map["goto"]->is_terminal = 1;
		sign_map[":"]->is_terminal = 1;
		sign_map["case"]->is_terminal = 1;
		sign_map["default"]->is_terminal = 1;
		sign_map["="]->is_terminal = 1;
		sign_map["+="]->is_terminal = 1;
		sign_map["-="]->is_terminal = 1;
		sign_map["*="]->is_terminal = 1;
		sign_map["/="]->is_terminal = 1;
		sign_map["%="]->is_terminal = 1;
		sign_map["<="]->is_terminal = 1;
		sign_map["<"]->is_terminal = 1;
		sign_map["=="]->is_terminal = 1;
		sign_map["!="]->is_terminal = 1;
		sign_map[">="]->is_terminal = 1;
		sign_map[">"]->is_terminal = 1;
		sign_map["+"]->is_terminal = 1;
		sign_map["-"]->is_terminal = 1;
		sign_map["*"]->is_terminal = 1;
		sign_map["/"]->is_terminal = 1;
		sign_map["%"]->is_terminal = 1;
		sign_map[""]->is_terminal = 1;
	}
	void build_first() {
		//终结符的first集合中只有它自身
		for (pair<string, Sign*> p : sign_map) {
			if (p.second->is_terminal) {
				first[p.first].insert(p.first);
			}
		}
		while (1) {
			bool change = 0;
			//反复扫描产生式，构造first集合
			for (auto p : productions) {
				string cur_sign = p.first;
				if (sign_map[cur_sign]->is_terminal) {
					if (p.second[0] != "") {//如果X->a...，则a属于first(X)集合
						if (first[cur_sign].find(p.second[0]) == first[cur_sign].end()) {
							first[cur_sign].insert(p.second[0]);
							change = 1;
						}
					} else if (p.second[0] == "" && p.second.size() == 1) {//如果X->ε，则ε属于first(X)集合
						if (first[cur_sign].find("") == first[cur_sign].end()) {
							first[cur_sign].insert("");
							change = 1;
						}
					}
				} else {
					//如果X->A，则first(X) += first(A)（ε除外）
					for (string _s : first[p.second[0]]) {
						if (_s != "") {
							if (first[cur_sign].find(_s) == first[cur_sign].end()) {
								first[cur_sign].insert(_s);
								change = 1;
							}
						}
					}
					int index = 0;
					while (index < p.second.size() && first[p.second[index]].find("") != first[p.second[index]].end()) {
						for (string _s : first[p.second[index]]) {
							if (_s != "") {
								if (first[cur_sign].find(_s) == first[cur_sign].end()) {
									first[cur_sign].insert(_s);
									change = 1;
								}
							}
						}
						index++;
					}
					if (index < p.second.size()) {
						for (string _s : first[p.second[0]]) {
							if (first[cur_sign].find(_s) == first[cur_sign].end()) {
								first[cur_sign].insert(_s);
								change = 1;
							}
						}
					} else {
						if (first[cur_sign].find("") == first[cur_sign].end()) {
							first[cur_sign].insert("");
							change = 1;
						}
					}
				}
			}
			if (change) {
				//如果本次循环有任何一个符号的first集合发生变化，则重新进行一遍扫描
				continue;
			} else {
				break;
			}
		}
	}
	void build_follow() {
		follow["program"].insert("#");
		while (1) {
			bool change = 0;
			for (auto p : productions) {
				string cur_sign = p.first;
				rg(i, 0, p.second.size(), 1) {
					auto next_first = merge_first(p.second, i + 1, p.second.size() - 1);
					if (i < p.second.size() - 1) {
						for (string _s : next_first) {
							if (_s != "") {
								if (follow[p.second[i]].find(_s) == follow[p.second[i]].end()) {
									follow[p.second[i]].insert(_s);
									change = 1;
								}
							}
						}
					}
					if (i == p.second.size() - 1 || i < p.second.size() - 1 && next_first.find("") != next_first.end()) {
						for (string _s : follow[cur_sign]) {
							if (follow[p.second[i]].find(_s) == follow[p.second[i]].end()) {
								follow[p.second[i]].insert(_s);
								change = 1;
							}
						}
					}
				}
			}
			if (change) {
				continue;
			} else {
				break;
			}
		}
	}
	void output_productions() {
		for (auto p : productions) {
			cout << p.first << " ->";
			for (auto x : p.second) {
				cout << " " << x;
			}
			puts("");
		}
	}
	void output_signs() {
		int cnt = 0;
		for (auto p : sign_map) {
			puts(p.first.c_str());
			if (p.second->is_terminal) {
				cnt++;
			}
		}
		printf("共有%d种符号，其中终结符%d种，非终结符%d种\n", sign_map.size(), cnt, sign_map.size() - cnt);
	}
	void output_first_set() {
		for (auto p : first) {
			printf("%s的first集合有%d个终结符：\n", p.first.c_str(), p.second.size());
			bool f = 0;
			for (string _s : p.second) {
				printf("%s\"%s\"", f ? ", " : "", _s.c_str());
				f = 1;
			}
			puts("");
		}
	}
	void output_follow_set() {
		for (auto p : follow) {
			printf("%s的follow集合有%d个终结符：\n", p.first.c_str(), p.second.size());
			bool f = 0;
			for (string _s : p.second) {
				printf("%s\"%s\"", f ? ", " : "", _s.c_str());
				f = 1;
			}
			puts("");
		}
	}
	void initiation(const char *lexical_filename/*, const char *intermediate_code_name*/) {
		//intermediate_code_generator = fopen(intermediate_code_name, "w");
		lexical_reader = fopen(lexical_filename, "r");
		attributer_reader = scanner::synthesized_attribute;
		while (!feof(lexical_reader)) {
			string left_sign = "";
			char c = 0;
			vector<string> right_signs;
			char _sign[128] = "";
			fscanf(lexical_reader, "%s -> ", _sign);
			if (sign_map.find(_sign) == sign_map.end()) {
				sign_map[_sign] = new Sign(_sign);
			}
			left_sign = _sign;
			do {
				_sign[0] = 0;
				fscanf(lexical_reader, "%[^#^ ]", _sign);
				if (_sign[0] == '`') {
					continue;
				}
				if (sign_map.find(_sign) == sign_map.end()) {
					sign_map[_sign] = new Sign(_sign);
				}
				right_signs.push_back(_sign);
				c = fgetc(lexical_reader);
			} while (c == ' ');
			productions.insert(make_pair(left_sign, right_signs));
			fgetc(lexical_reader);
		}
		fclose(lexical_reader);
		set_terminals();
		build_first();
		build_follow();
	}
	void output_first_set(string s) {
		printf("%s的first集合有%d个终结符：\n", s.c_str(), first[s].size());
		bool f = 0;
		for (string _s : first[s]) {
			printf("%s\"%s\"", f ? ", " : "", _s.c_str());
			f = 1;
		}
		puts("");
	}
	void output_follow_set(string s) {
		printf("%s的follow集合有%d个终结符：\n", s.c_str(), follow[s].size());
		bool f = 0;
		for (string _s : follow[s]) {
			printf("%s\"%s\"", f ? ", " : "", _s.c_str());
			f = 1;
		}
		puts("");
	}
	void unupdatable(string var) {
		printf("'%s' must be updatable left-value.", var.c_str());
		exit(0);
	}
	void invalid_conversion(string t1, string t2) {
		printf("error: invalid conversion from type '%s' to '%s'.\a", t2.c_str(), t1.c_str());
		exit(0);
	}
	void illegal_use() {
		printf("error: illegal use of symbol '%s'.\a", cur_sign.c_str());
		outputTable();
		exit(0);
	}
	void non_definition(string definition) {
		printf("error: '%s' has not been defined.\a", definition.c_str());
		exit(0);
	}
	void dup_definition(string definition) {
		printf("error: previous definition of '%s'.\a", definition.c_str());
		exit(0);
	}
	bool in_first(string cur_sign, string _str) {
		return first[_str].find(cur_sign) != first[_str].end();
	}
	bool in_first(string cur_sign, unordered_set<string> s) {
		return s.find(cur_sign) != s.end();
	}
	bool in_first(string sign, int cnt, ...) {
		va_list ap;
		va_start(ap, cnt);
		vector<string> v;
		while (cnt--) {
			string new_sign = va_arg(ap, char*);
			v.push_back(new_sign);
		}
		va_end(ap);
		return in_first(sign, merge_first(v));
	}
	bool in_follow(string cur_sign, string _str) {
		return follow[_str].find(cur_sign) != follow[_str].end();
	}
	Sign* program();
	void parse(const char *filename) {
		symbol_table_space::initiation();
		//    create_symbol_table();
		lexical_reader = fopen(filename, "r");
		char buffer[128];
		fscanf(lexical_reader, "%s", buffer);
		cur_sign = string(buffer);
		match("#");
		Sign* root = 0;
		if (in_first(cur_sign, "program")) {
			root = program();
			threeAddrCodes = root->code;
		} else if (in_first("", "program") && in_follow(cur_sign, "program")) {

		} else {
			illegal_use();
		}
		match("#");
		//for (Quadruples *q : threeAddrCodes) {
		//	printf("%s\n", q->toString().c_str());
		//}
		//three_addr_codes.clear();
		fclose(lexical_reader);
		//fclose(intermediate_code_generator);
		//puts("Syntax analysis successful.");
	}
	Sign* declaration_list();
	Sign* program() {
		Sign* ret = new Sign("program");
		if (in_first(cur_sign, "declaration-list")) {
			//        enter_scope();
			auto node = declaration_list();
			ret->child.push_back(node);
			ret->code = node->code;
			//        exit_scope();
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* declaration();
	Sign* declaration_list() {
		Sign* ret = new Sign("declaration-list");
		if (in_first(cur_sign, 2, "declaration", "declaration-list")) {
			auto node = declaration();
			ret->child.push_back(node);
			ret->code = node->code;
			//outputTable();
			//system("pause");
			node = declaration_list();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);
		} else if (in_first("", "declaration-list") && in_follow(cur_sign, "declaration-list")) {
			/*do nothing*/
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* declaration_feature(string, string);
	Sign* type_specifier();
	Sign* ID();
	Sign* declaration() {
		Sign* ret = new Sign("declaration");
		if (in_first(cur_sign, 3, "type-specifier", "ID", "declaration-feature")) {
			auto node = type_specifier();
			ret->child.push_back(node);
			string type = node->attributes["type"];
			if (type != "i" && type != "v") {
				ret->code = node->code;
				if (cur_sign == ";") {
					//struct Student{int id; int age;};
					match(";");
				} else {
					//struct Student{int id; int age;}s;
					node = ID();
					ret->child.push_back(node);
					string id = node->attributes["id"];
					if (lookup(id).first != -1) {
						dup_definition(id);
					}
					node = declaration_feature(type, id);
					ret->child.push_back(node);
					for (auto x : node->code)ret->code.push_back(x);
				}
			} else {
				//int id;或Student s;或void main(){...}
				node = ID();
				ret->child.push_back(node);
				string id = node->attributes["id"];
				if (lookup(id).first != -1) {
					dup_definition(id);
				}
				node = declaration_feature(type, id);
				ret->child.push_back(node);
				for (auto x : node->code)ret->code.push_back(x);
			}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* struct_specifier();
	Sign* type_specifier() {
		Sign* ret = new Sign("type-specifier");
		if (cur_sign == "int") {
			match("int");
			ret->attributes["type"] = "i";
		} else if (cur_sign == "void") {
			ret->attributes["type"] = "v";
			match("void");
		} else if (in_first(cur_sign, "struct-specifier")) {
			auto node = struct_specifier();
			ret->child.push_back(node);
			string type = node->attributes["type"];
			ret->attributes["type"] = type;
			ret->code = node->code;
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* ID() {
		auto ret = new Sign("ID");
		match("ID");
		/*char buffer[128];
		fscanf(attribute_reader, "%s", buffer);*/
		string buffer = attributer_reader.front();
		attributer_reader.pop();
		//printf("ID综合属性为%s\n", buffer.c_str());
		ret->attributes["id"] = buffer;
		return ret;
	}
	Sign* NUM() {
		auto ret = new Sign("NUM");
		match("NUM");
		/*char buffer[128];
		fscanf(attribute_reader, "%s", buffer);*/
		string buffer = attributer_reader.front();
		attributer_reader.pop();
		//printf("NUM综合属性为%s\n", buffer.c_str());
		ret->attributes["num"] = buffer;
		ret->attributes["type"] = "i";
		return ret;
	}
	Sign* variable_feature(string, string);
	Sign* function_feature(string, string);
	Sign* declaration_feature(string type, string id) {
		auto ret = new Sign("declaration-feature)");
		if (in_first(cur_sign, "variable-feature")) {
			auto node = variable_feature(type, id);
			ret->child.push_back(node);
		} else if (in_first(cur_sign, "function-feature")) {
			clearControlVar();
			auto node = function_feature(type, id);
			ret->child.push_back(node);
			ret->code = node->code;
			cur_function = "";
			clearControlVar();
		} else {
			illegal_use();
		}
		return ret;
	}
	namespace constant_expression {
		int mul_exp();
		int add_exp() {
			int ans = 0;
			ans = mul_exp();
			while (cur_sign == "+" || cur_sign == "-") {
				if (cur_sign == "+") {
					match("+");
					ans += mul_exp();
				} else {
					match("-");
					ans -= mul_exp();
				}
			}
			return ans;
		}
		int pri_exp();
		int mul_exp() {
			int ans = 0;
			ans = pri_exp();
			while (cur_sign == "*" || cur_sign == "/") {
				if (cur_sign == "*") {
					match("*");
					ans *= pri_exp();
				} else {
					match("/");
					ans /= pri_exp();
				}
			}
			return ans;
		}
		int pri_exp() {
			int ans = 0;
			bool neg = 0;
			if (cur_sign == "-") {
				match("-");
				neg = 1;
			}
			if (cur_sign == "(") {
				match("(");
				ans = add_exp();
				match(")");
			} else {
				ans = stoi(NUM()->attributes["num"]);
			}
			if (neg) {
				ans = -ans;
			}
			return ans;
		}
	}
	Sign* variable_feature(string type, string id) {
		auto ret = new Sign("variable-feature");
		if (cur_sign == ";") {
			match(";");
			//在当前作用域中查找此ID，若能找到说明此ID先前已经被定义过，不能重复定义
			if (symbolTable.size() > curScope && find_if(symbolTable[curScope].begin(), symbolTable[curScope].end(), [=](auto s) {
				return s->name == id;
			}) != symbolTable[curScope].end()) {
				dup_definition(id);
			}
			addSymbol(id, type, KIND::VARIABLE, 0);
		} else if (cur_sign == "[") {//ID后面跟着[，说明要声明的是一个数组
			//在当前作用域中查找此ID，若能找到说明此ID先前已经被定义过，不能重复定义
			if (symbolTable.size() > curScope && find_if(symbolTable[curScope].begin(), symbolTable[curScope].end(), [=](auto s) {
				return s->name == id;
			}) != symbolTable[curScope].end()) {
				dup_definition(id);
			}
			match("[");
			int array_size = constant_expression::add_exp();
			match("]");
			match(";");//只支持声明一维数组

			if (type != "i") {
				//结构体数组，类型为P$type.len$type
				addSymbol(id, "P" + to_string(type.length()) + type, KIND::POINTER, array_size, false);
			} else {
				//整型数组，类型为Pi
				addSymbol(id, "P" + type, KIND::POINTER, array_size, false);
			}
		} else {
			illegal_use();
		}
		return ret;
	}

	Sign* local_declarations();
	Sign* statement_list();
	Sign* extra_statement();
	Sign* parameters();
	Sign* function_feature(string type, string id) {
		auto ret = new Sign("function-feature");
		ret->code.push_back(new Quadruples("proc", "", "", id));
		auto procpos = new SignList(&ret->code.back()->result);
		addSymbol(id, type, KIND::FUNCTION, 0);//目前不知道参数的具体情况，暂时置info为0，id为函数名
		if (cur_sign == "(") {
			match("(");
			enterScope();
			int sub_field = curScope;
			auto node = parameters();
			ret->child.push_back(node);
			auto pos = lookup(id);
			string name = symbolTable[pos.first][pos.second]->name;
			attachTypes(name);//把参数加到函数名后面，用name暂存这个函数名
			if (lookup(name).first != -1) {
				dup_definition(name);
			}
			symbolTable[pos.first][pos.second]->name = name;//更新符号表中的函数名
			symbolTable[pos.first][pos.second]->info = sub_field;//知道参数后，改变符号表中的info值
			match(")");
			parLen[name] = symbolTable[sub_field].size();//保存函数参数个数
			cur_function = name;
			//        void compound_statement();
			//        compound_statement();
			//函数内部的局部变量与参数属于同一作用域，不应该进入新作用域。
			//而调用compound_statement()时会自动进入新作用域，因此不能直接调用compound_statement()。
			match("{");
			node = local_declarations();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);
			node = statement_list();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);
			node = extra_statement();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);
			match("}");
			exitScope();
			SignList::backPatch(procpos, name);
			SignList::delList(procpos);
			ret->code.push_back(new Quadruples("endp", "", "", name));
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign *struct_declaration_compound();
	Sign* struct_specifier() {
		auto ret = new Sign("struct-specifier");
		if (cur_sign == "struct") {
			match("struct");
			Sign *node = ID();
			ret->child.push_back(node);
			//type=$n$name
			string type = node->attributes["id"];
			ret->attributes["type"] = type;
			ret->code.push_back(new Quadruples("struct", "", "", type));
			auto pos = lookup(type);
			if (pos.first != -1) {
				//struct-id类型重复定义
				dup_definition(type);
			}
			//定义结构体类型，要先把struct-id加入符号表，成员变量暂时没有被识别，info先置为-1
			addSymbol(type, "struct-id", KIND::STRUCT_ID, -1);
			node = struct_declaration_compound();
			ret->child.push_back(node);
			for (auto x : node->code) {
				ret->code.push_back(x);
			}
			//定义结构体类型，回填成员变量域
			pos = lookup(type);
			int sub_scope = stoi(node->attributes["sub_scope"]);
			symbolTable[pos.first][pos.second]->info = sub_scope;
			ret->code.push_back(new Quadruples("ends", "", "", type));
		} else if (cur_sign == "ID") {
			Sign *node = ID();
			string type = node->attributes["id"];
			ret->attributes["type"] = type;
			if (lookup(type).first == -1) {
				//struct-id类型未定义
				non_definition(type);
			}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* struct_declaration_list();
	Sign* struct_declaration_compound() {//返回成员变量域在符号表中的位置
		auto ret = new Sign("struct-declaration-compound");
		if (cur_sign == "{") {
			//定义一个结构体类型
			match("{");
			enterScope();
			auto node = struct_declaration_list();
			ret->child.push_back(node);
			ret->code = node->code;
			int sub_scope = curScope;
			ret->attributes["sub_scope"] = to_string(sub_scope);
			exitScope();
			match("}");
			//        match(";");
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* struct_declaration_list() {
		auto ret = new Sign("struct-declaration-list");
		if (cur_sign == "int") {
			match("int");
			auto node = ID();
			ret->child.push_back(node);
			string id = node->attributes["id"];
			match(";");
			addSymbol(id, "i", KIND::VARIABLE, 0);
			ret->code.push_back(new Quadruples("int", "", "", id));
			node = struct_declaration_list();
			ret->child.push_back(node);
			for (auto x : node->code) {
				ret->code.push_back(x);
			}
		} else if (in_first("", "struct-declaration-list") && in_follow(cur_sign, "struct-declaration-list")) {
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* parameter_list();
	Sign* parameters() {
		auto ret = new Sign("parameters");
		if (cur_sign == "void") {
			//void f(void)的参数个数为0
			match("void");
		} else if (in_first(cur_sign, "parameter-list")) {
			auto node = parameter_list();
			ret->child.push_back(node);
		} else if (in_first("", "parameters") && in_follow(cur_sign, "parameters")) {
			//void f()的参数个数为0
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* parameter();
	Sign* extra_parameter_list();
	Sign* parameter_list() {
		auto ret = new Sign("parameter-list");
		if (in_first(cur_sign, 2, "parameter", "extra-parameter-list")) {
			auto node = parameter();
			ret->child.push_back(node);
			node = extra_parameter_list();
			ret->child.push_back(node);
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* is_array_parameter();
	Sign* parameter() {
		auto ret = new Sign("parameter");
		if (in_first(cur_sign, "type-specifier")) {
			auto node = type_specifier();
			ret->child.push_back(node);
			string type = node->attributes["type"];
			node = ID();
			ret->child.push_back(node);
			string id = node->attributes["id"];
			node = is_array_parameter();
			ret->child.push_back(node);
			string array_len = node->attributes["is-array-parameter"];
			//        puts("压入一个参数");
			
			if (array_len == "0") {
				//参数为单变量
				addSymbol(id, type, KIND::VARIABLE, 0);
				if (type != "i")type.insert(0, to_string(type.length()));
			} else {
				//参数为指针
				if (type == "i") {
					addSymbol(id, "P" + type, KIND::POINTER, INT_MAX);
				} else {
					addSymbol(id, "P" + to_string(type.length()) + type, KIND::POINTER, INT_MAX);
				}
				if (type != "i")type.insert(0, to_string(type.length()));
				type = "P" + type;
			}
			par_types.push_back(type);
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* extra_parameter_list() {//返回从当前位置到参数列表末尾的参数的个数
		auto ret = new Sign("extra-parameter-list");
		if (cur_sign == ",") {
			match(",");
			auto node = parameter();
			ret->child.push_back(node);
			node = extra_parameter_list();
			ret->child.push_back(node);
		} else if (in_first("", "extra-parameter-list") && in_follow(cur_sign, "extra-parameter-list")) {

		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* is_array_parameter() {//如果参数为数组，返回1，否则返回0
		auto ret = new Sign("is-array-parameter");
		if (cur_sign == "[") {
			match("[");
			match("]");
			ret->attributes["is-array-parameter"] = "1";
		} else if (in_first("", "is-array-parameter") && in_follow(cur_sign, "is-array-parameter")) {
			ret->attributes["is-array-parameter"] = "0";
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* local_declarations();
	Sign* statement_list();
	Sign* extra_statement();
	Sign* compound_statement() {
		auto ret = new Sign("compound-statement");
		if (cur_sign == "{") {
			match("{");
			enterScope();
			auto node = local_declarations();
			ret->child.push_back(node);
			ret->code = node->code;//C.code=L.code
			node = statement_list();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);//C.code+=S.code
			ret->breakList = SignList::mergeList(ret->breakList, node->breakList);
			ret->continueList = SignList::mergeList(ret->continueList, node->continueList);
			node = extra_statement();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);//C.code=eS.code
			ret->breakList = SignList::mergeList(ret->breakList, node->breakList);
			ret->continueList = SignList::mergeList(ret->continueList, node->continueList);
			exitScope();
			match("}");
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* local_declarations();
	Sign* statement_list();
	Sign* extra_statement();
	Sign* extra_statement() {
		auto ret = new Sign("extra-statement");
		if (in_first(cur_sign, 3, "local-declarations", "statement-list", "extra-statement")) {
			auto node = local_declarations();
			ret->child.push_back(node);
			ret->code = node->code;
			node = statement_list();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);
			node = extra_statement();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);
		} else if (in_first("", "extra-statement") && in_follow(cur_sign, "extra-statement")) {

		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* local_declarations() {
		auto ret = new Sign("local-declarations");
		if (cur_sign == "ID") {
			//当前单词是ID，而下一单词可能是冒号，表示标记语句(statement)，此时没有声明任何新变量，应当推导为ε
			if (get_next_sign() == ":") {
				return ret;
			} else {
				//当前单词是ID，但可能并不是结构体类型标识符，表示表达式语句(statement)，此时没有声明任何新变量，应当推导为ε
				auto pos = lookup(getCurAttr());
				if (pos.first == -1 || symbolTable[pos.first][pos.second]->kind != KIND::STRUCT_ID) {
					return ret;
				}
			}
		}
		if (in_first(cur_sign, "type-specifier")) {
			auto node = type_specifier();
			ret->child.push_back(node);
			string type = node->attributes["type"];
			if (type != "i" && type != "v") {
				ret->code = node->code;
			}
			node = ID();
			ret->child.push_back(node);
			string id = node->attributes["id"];
			node = variable_feature(type, id);
			ret->child.push_back(node);
			node = local_declarations();
			ret->child.push_back(node);
		} else if (in_first("", "local-declarations") && in_follow(cur_sign, "local-declarations")) {
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* statement();
	Sign* statement_list() {
		auto ret = new Sign("statement-list");
		if (in_first(cur_sign, 2, "statement", "statement-list")) {
			//如果以ID开头，可能是声明某个结构体变量，当前应该推导为declaration而非statement
			if (cur_sign == "ID") {
				auto pos = lookup(getCurAttr());
				if (pos.first != -1 && symbolTable[pos.first][pos.second]->kind == KIND::STRUCT_ID) {
					//不执行任何语义动作，直接返回。
					return ret;
				}
			}
			auto node = statement();
			ret->child.push_back(node);
			ret->code = node->code;
			ret->breakList = SignList::mergeList(ret->breakList, node->breakList);
			ret->continueList = SignList::mergeList(ret->continueList, node->continueList);
			node = statement_list();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);
			ret->breakList = SignList::mergeList(ret->breakList, node->breakList);
			ret->continueList = SignList::mergeList(ret->continueList, node->continueList);
		} else if (in_first("", "statement-list") && in_follow(cur_sign, "statement-list")) {
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* expression_statement();
	Sign* selection_statement();
	Sign* labeled_statement();
	Sign* iteration_statement();
	Sign* jump_statement();
	Sign* case_statement();
	Sign* statement() {
		auto ret = new Sign("statement");
		if (in_first(cur_sign, "compound-statement")) {
			auto node = compound_statement();
			ret->child.push_back(node);
			ret->code = node->code;
			ret->breakList = node->breakList;
			ret->continueList = node->continueList;
		} else if (in_first(cur_sign, "expression-statement") && get_next_sign() != ":") {//以ID开头的语句可能是表达式语句ID=...或标号语句ID:...
			auto node = expression_statement();
			ret->child.push_back(node);
			ret->code = node->code;
		} else if (in_first(cur_sign, "selection-statement")) {
			auto node = selection_statement();
			ret->child.push_back(node);
			ret->code = node->code;
			ret->breakList = node->breakList;
			ret->continueList = node->continueList;
		} else if (in_first(cur_sign, "labeled-statement")) {
			auto node = labeled_statement();
			ret->child.push_back(node);
			ret->code = node->code;
		} else if (in_first(cur_sign, "iteration-statement")) {
			auto node = iteration_statement();
			ret->child.push_back(node);
			ret->code = node->code;
		} else if (in_first(cur_sign, "jump-statement")) {
			auto node = jump_statement();
			ret->child.push_back(node);
			ret->code = node->code;
			ret->breakList = node->breakList;
			ret->continueList = node->continueList;
		} else if(in_first(cur_sign, "case-statement")){
			auto node = case_statement();
			ret->child.push_back(node);
			ret->breakList = node->breakList;
			ret->continueList = node->continueList;
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* optional_expression();
	Sign* expression_statement() {
		auto ret = new Sign("expression-statement");
		if (in_first(cur_sign, "optional-expression")) {
			auto node = optional_expression();
			ret->child.push_back(node);
			ret->code = node->code;
			match(";");
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* expression();
	Sign* optional_expression() {
		auto ret = new Sign("optional-expression");
		if (in_first(cur_sign, "expression")) {
			auto node = expression();
			ret->child.push_back(node);
			ret->code = node->code;
			ret->attributes["addr"] = node->attributes["addr"];
		} else if (in_first("", "optional-expression") && in_follow(cur_sign, "optional-expression")) {
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* if_branch();
	Sign* else_branch();
	Sign* switch_branch();
	Sign* selection_statement() {
		auto ret = new Sign("selection-statement");
		if (in_first(cur_sign, 2, "if-branch", "else-branch")) {
			auto left = if_branch();
			ret->child.push_back(left);
			ret->code = left->code;//S.code = I.code
			auto right = else_branch();
			ret->child.push_back(right);
			SignList::backPatch(left->exit, right->attributes["label"]);//backpatch(I.exit, E.label)
			SignList::delList(left->exit);
			if (right->attributes["has-else"] == "1") {//if(has-else):
				string outlabel = createLabel();//outlabel=newlabel()
				ret->code.push_back(new Quadruples("jmp", "", "", outlabel));//S.code+=(goto outlabel)
				for (auto x : right->code)ret->code.push_back(x);//S.code += E.code
				ret->code.push_back(new Quadruples(outlabel, "", "", ""));//S.code+=(outlabel: )
			} else {
				for (auto x : right->code)ret->code.push_back(x);//S.code += E.code
			}
			ret->breakList = SignList::mergeList(left->breakList, right->breakList);
			ret->continueList = SignList::mergeList(left->continueList, right->continueList);
		} else if (in_first(cur_sign, "switch-branch")) {
			if (!jumpable) {
				jumpable = 1;
				continuable = 0;
				auto node = switch_branch();
				ret->child.push_back(node);
				continuable = 1;
				jumpable = 0;
				for (auto x : node->code)ret->code.push_back(x);//Se.code+=S.code
			} else {
				auto node = switch_branch();
				ret->child.push_back(node);
				for (auto x : node->code)ret->code.push_back(x);//Se.code+=S.code
			}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* expression();
	Sign* if_branch() {
		auto ret = new Sign("if-branch");
		if (cur_sign == "if") {
			match("if");
			match("(");
			auto expNode = expression();
			ret->child.push_back(expNode);
			match(")");
			auto stmt = statement();
			ret->child.push_back(stmt);
			ret->code = expNode->code;//I.code = C.code
			ret->code.push_back(new Quadruples("jz", expNode->attributes["addr"], "", "?"));//I.code += (jz, C.add, $, ?)
			SignList::suspend(ret->exit, &ret->code.back()->result);//I.exit = ?
			for (auto x : stmt->code)ret->code.push_back(x);//I.code += S.code
			ret->breakList = stmt->breakList;
			ret->continueList = stmt->continueList;
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* else_branch() {
		auto ret = new Sign("else-branch");
		if (cur_sign == "else") {
			match("else");
			auto node = statement();
			ret->child.push_back(node);
			ret->attributes["has-else"] = "1";//E.has-else = 1
			ret->attributes["label"] = createLabel();//E.label = newlabel
			ret->code.push_back(new Quadruples(getLabel(), "", "", ""));//E.code = (E.label: )
			for (auto x : node->code)ret->code.push_back(x);//E.code += S.code
			ret->breakList = node->breakList;
			ret->continueList = node->continueList;
		} else if (in_first("", "else-branch") && in_follow(cur_sign, "else-branch")) {
			//if语句没有else分支，此处推导为epsilon
			ret->attributes["has-else"] = "0";//E.has-else = 0
			ret->attributes["label"] = createLabel();//E.label = newlabel
			ret->code.push_back(new Quadruples(getLabel(), "", "", ""));//E.code = (getlabel, $, $, $)
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* conditional_expression();
	Sign* switch_branch() {
		auto ret = new Sign("switch-branch");
		if (cur_sign == "switch") {
			match("switch");
			match("(");
			string testLabel = createLabel();
			string nextLabel = createLabel();
			string tmp = createTemp();
			auto expNode = conditional_expression();
			ret->child.push_back(expNode);
			match(")");
			match("{");
			enterScope();
			ret->code = expNode->code;//S.code+=E.code
			string expAddr = expNode->attributes["addr"];
			ret->code.push_back(new Quadruples("jmp", "", "", testLabel));//S.code+=(goto testLabel)
			vector<Quadruples*> tester;
			while (cur_sign == "case" || cur_sign == "default") {
				auto caseNode = case_statement();
				ret->child.push_back(caseNode);
				for (auto x : caseNode->code)ret->code.push_back(x);//S.code+=C.code
				ret->breakList = SignList::mergeList(ret->breakList, caseNode->breakList);//S.breakList=mergeList(S.breakList, C.breakList)
				if (caseNode->attributes["keyword"] == "C") {
					tester.push_back(new Quadruples("-", expNode->attributes["addr"], caseNode->attributes["addr"], tmp));//T.code+=(tmp=E.addr-C.addr)
					tester.push_back(new Quadruples("jz", tmp, "", caseNode->attributes["label"]));//T.code+=(if tmp==0 goto C.label)
				} else {
					tester.push_back(new Quadruples("jmp", "", "", caseNode->attributes["label"]));//T.code+=(goto C.label)
				}
			}
			ret->code.push_back(new Quadruples(testLabel, "", "", ""));//S.code+=(testLabel: )
			for (auto x : tester)ret->code.push_back(x);//S.code+=T.code
			ret->code.push_back(new Quadruples(nextLabel, "", "", ""));//S.code+=(nextLabel: )
			SignList::backPatch(ret->breakList, nextLabel);//backpatch(S.breakList, nextLabel)
			SignList::delList(ret->breakList);
			match("}");
			exitScope();
			if (cur_sign == ";") {
				match(";");
			}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* expression();
	Sign* iteration_statement() {
		auto ret = new Sign("iteration-statement");
		if (cur_sign == "while") {
			match("while");
			match("(");
			auto node = expression();
			ret->child.push_back(node);
			string looplabel = createLabel();//looplabel=newlabel
			string exitLabel = createLabel();//exitlabel=newlabel
			ret->code.push_back(new Quadruples(looplabel, "", "", ""));//I.code=(looplabel, $, $, $)
			for (auto x : node->code)ret->code.push_back(x);//I.code+=C.code
			ret->code.push_back(new Quadruples("jz", node->attributes["addr"], "", exitLabel));//I.code+=(jz, C.addr, $, exitlabel)
			match(")");
			if (!jumpable) {
				jumpable = 1;//只有在jumpable==1时才能有statement -> break ; | continue ;
				node = statement();
				jumpable = 0;
			} else {
				node = statement();
			}
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);//I.code+=S.code
			ret->code.push_back(new Quadruples("jmp", "", "", looplabel));//I.code+=(jmp, $, $, looplabel)
			ret->code.push_back(new Quadruples(exitLabel, "", "", ""));//I,code+=(exitlabel, $, $, $)
			SignList::backPatch(node->breakList, exitLabel);//backpatch(S.breakList, exitLabel)
			SignList::delList(node->breakList);
			SignList::backPatch(node->continueList, looplabel);//backpatch(S.continueList, loopLabel)
			SignList::delList(node->continueList);
		} else if (cur_sign == "for") {
			match("for");
			match("(");
			auto node = optional_expression();
			ret->child.push_back(node);
			match(";");
			ret->code = node->code;//I.code=P.code
			node = optional_expression();
			ret->child.push_back(node);
			match(";");
			string expAddr = node->attributes["addr"];
			string startlabel = createLabel();//startlabel=newlabel
			string loopLabel = createLabel();//looplabel=newlabel
			string exitLabel = createLabel();//exitlabel=newlabel
			ret->code.push_back(new Quadruples(startlabel, "", "", ""));//I.code+=(startlabel, $, $, $)
			if (expAddr != "") {//if(not(B->ε))
				for (auto x : node->code)ret->code.push_back(x);//I.code+=B.code
				ret->code.push_back(new Quadruples("jz", node->attributes["addr"], "", exitLabel));//I.code+=(jz, B.addr, $, exitlabel)
			} else {
				ret->code.push_back(new Quadruples("jz", "1", "", exitLabel));//I.code+=(jz, 1, $, exitlabel)
			}
			auto loopnode = optional_expression();
			ret->child.push_back(loopnode);
			match(")");
			//I.code+=S.code
			if (!jumpable) {
				jumpable = 1;
				node = statement();
				ret->child.push_back(node);
				for (auto x : node->code)ret->code.push_back(x);
				jumpable = 0;
			} else {
				node = statement();
				ret->child.push_back(node);
				for (auto x : node->code)ret->code.push_back(x);
			}
			SignList::backPatch(node->breakList, exitLabel);//backpatch(S.breaklist, exitlabel)
			SignList::delList(node->breakList);
			SignList::backPatch(node->continueList, loopLabel);//backpatch(S.continuelist, looplabel)
			SignList::delList(node->continueList);
			ret->code.push_back(new Quadruples(loopLabel, "", "", ""));//I.code+=(looplabel, $, $, $)
			for (auto x : loopnode->code)ret->code.push_back(x);//I.code+=L.code
			ret->code.push_back(new Quadruples("jmp", "", "", startlabel));//I.code+=(jmp, $, $, startlabel)
			ret->code.push_back(new Quadruples(exitLabel, "", "", ""));//I.code+=(exitlabel, $, $, $)
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* jump_statement() {
		auto ret = new Sign("jump-statement");
		if (cur_sign == "return") {
			match("return");
			auto node = optional_expression();
			ret->child.push_back(node);
			match(";");
			ret->code = node->code;
			ret->code.push_back(new Quadruples("ret", "", "", node->attributes["addr"]));
		} else if (cur_sign == "break" && jumpable) {
			match("break");
			match(";");
			ret->code.push_back(new Quadruples("jmp", "", "", ""));
			SignList::suspend(ret->breakList, &ret->code.back()->result);
		} else if (cur_sign == "continue" && jumpable && continuable) {
			match("continue");
			match(";");
			ret->code.push_back(new Quadruples("jmp", "", "", ""));
			SignList::suspend(ret->continueList, &ret->code.back()->result);
		} else if (cur_sign == "goto") {
			//goto保留为迷你C的关键字，但在语法上不允许出现，这与Java的做法一致
			illegal_use();
			//match("goto");
			//auto node = ID();
			//ret->childs.push_back(node);
			//match(";");
			//string id = node->attributes["id"];
			//auto pos = lookup(id);
			//if (pos.first == -1) {
			//	non_definition(id);
			//} else {
			//	//标号label已定义
			//	auto label = symbol_table[pos.first][pos.second];
			//	ret->code.push_back(new Quadruples("jmp", "", "", "L" + to_string(label->info)));
			//}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* case_statement() {
		auto ret = new Sign("case-statement");
		if (cur_sign == "case") {
			match("case");
			auto node = NUM();
			ret->child.push_back(node);
			ret->attributes["addr"] = node->attributes["num"];
			match(":");
			node = statement();
			ret->child.push_back(node);
			ret->breakList = node->breakList;
			ret->attributes["keyword"] = "C";
			ret->attributes["label"] = createLabel();
			ret->code.push_back(new Quadruples(ret->attributes["label"], "", "", ""));
			for (auto x : node->code)ret->code.push_back(x);
		} else if (cur_sign == "default") {
			match("default");
			match(":");
			auto node = statement();
			ret->child.push_back(node);
			ret->breakList = node->breakList;
			ret->attributes["keyword"] = "D";
			ret->attributes["label"] = createLabel();
			ret->code.push_back(new Quadruples(ret->attributes["label"], "", "", ""));
			for (auto x : node->code)ret->code.push_back(x);
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* labeled_statement() {
		auto ret = new Sign("labeled-statement");
		if (cur_sign == "ID") {
			auto node = ID();
			//string id = node->attributes["id"];
			//auto pos = lookup(id);
			//if (pos.first != -1) {
			//	dup_definition(id);
			//}
			//string label = createLabel();
			//add_symbol(id, "label", KIND::LABEL, stoi(label.substr(1)));
			ret->child.push_back(node);
			match(":");
			//ret->code.push_back(new Quadruples(label, "", "", ""));
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* assignment_expression();
	Sign* conditional_expression();
	Sign* expression() {//返回值为表达式返回类型
		auto ret = new Sign("expression");
		//赋值表达式和条件表达式的左边都有可能只有一个变量符号，因此要根据后面有没有赋值符来判断用哪个文法符号进行扩展
		int offset = 1;
		bool assexp = 0;
		do {
			string next_sign = sign_advanced(offset);
			if (in_first(next_sign, "ass-operator")) {
				assexp = 1;
				break;
			}
			if (in_first(next_sign, "rel-operator") || next_sign == ";" || next_sign == ")" || next_sign == "#") {
				break;
			}
			offset++;
		} while (1);
		if (assexp && in_first(cur_sign, "assignment-expression")) {
			auto node = assignment_expression();
			ret->child.push_back(node);
			string type = node->attributes["type"];
			ret->attributes["type"] = type;
			ret->code = node->code;
			ret->attributes["addr"] = node->attributes["addr"];
		} else if (in_first(cur_sign, "conditional-expression")) {
			auto node = conditional_expression();
			ret->child.push_back(node);
			string type = node->attributes["type"];
			ret->attributes["type"] = type;
			ret->code = node->code;
			ret->attributes["addr"] = node->attributes["addr"];
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* variable(bool);
	Sign* ass_operator();
	Sign* expression();
	Sign* assignment_expression() {
		auto ret = new Sign("assignment-expression");
		if (in_first(cur_sign, 3, "variable", "ass-operator", "conditional-expression")) {
			auto node = variable(1);
			auto var = node;
			ret->child.push_back(node);
			string var_type = node->attributes["type"];
			node = ass_operator();
			ret->child.push_back(node);
			string ass_op = node->attributes["ass-operator"];
			node = conditional_expression();
			ret->child.push_back(node);
			string exp_type = node->attributes["type"];
			if (var_type != exp_type) {//类型不一致时，应当尝试类型转换，但在此语言中只有int, void, struct类型，所以无法进行转换
				invalid_conversion(var_type, exp_type);
			}
			if (ass_op == "=") {
				ret->code = node->code;
				ret->code.push_back(new Quadruples("=", node->attributes["addr"], "", var->attributes["addr"]));
				if (var->attributes["arrayFlag"] == "P") {
					swap(ret->code.back()->opnd1, ret->code.back()->opnd2);
					ret->code.insert(prev(ret->code.end()), var->code.begin(), var->code.end());
					string &op = ret->code.back()->op;
					op.insert(0, "[]");
					string &opnd1 = ret->code.back()->opnd1;
					opnd1 = var->attributes["expAddr"];
				}
				if (var->attributes["fieldFlag"] != "") {
					string &op = ret->code.back()->op;
					op.insert(op.length() - 1, "." + var->attributes["fieldFlag"]);
				}
			} else if (ass_op.length() == 2 && ass_op[1] == '=') {
				illegal_use();
				ret->code = node->code;
				string tmp = createTemp();
				int tmpIdx = tempCnt;
				ret->code.push_back(new Quadruples("=", var->attributes["addr"], "", tmp));
				//不支持运算符重载，所以临时变量类型必为i。如果支持运算符重载，类型与左值类型一致。
				tempType[cur_function][tmpIdx] = var->attributes["type"];
				if (var->attributes["arrayFlag"] == "P") {
					//swap(ret->code.back()->opnd1, ret->code.back()->opnd2);
					//ret->code.insert(prev(ret->code.end()), var->code.begin(), var->code.end());
					string &op = ret->code.back()->op;
					op += "[]";
					string &opnd2 = ret->code.back()->opnd2;
					opnd2 = var->attributes["expAddr"];
				}
				if (var->attributes["fieldFlag"] != "") {
					string &op = ret->code.back()->op;
					op += "." + var->attributes["fieldFlag"];
				}
				string result = createTemp();
				tmpIdx = tempCnt;
				ret->code.push_back(new Quadruples(ass_op.substr(0, 1), tmp, node->attributes["addr"], result));
				ret->code.push_back(new Quadruples("=", result, "", var->attributes["addr"]));
				//类型与左值保持一致
				tempType[cur_function][tmpIdx] = var->attributes["type"];
			}
			ret->attributes["addr"] = var->attributes["addr"];
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* is_array();
	Sign* is_field();
	Sign* variable(bool leftv) {
		auto ret = new Sign("variable");
		if (cur_sign == "ID") {
			auto varNode = ID();
			ret->child.push_back(varNode);
			string id = varNode->attributes["id"];
			auto pos = lookup(id);
			if (pos.first == -1) {
				non_definition(id);
			}
			auto arraynode = is_array();
			ret->child.push_back(arraynode);
			string arrayFlag = arraynode->attributes["arrayFlag"];
			ret->attributes["arrayFlag"] = arrayFlag;
			auto fieldnode = is_field();
			ret->child.push_back(fieldnode);
			string fieldFlag = fieldnode->attributes["fieldFlag"];
			ret->attributes["fieldFlag"] = fieldFlag;
			if (!leftv) {
				//变量是右值，被引用
				string tmp = createTemp();
				int tmpIdx = tempCnt;
				ret->attributes["addr"] = tmp;
				ret->code.push_back(new Quadruples("=", get_entry(pos), "", tmp));
				if (arrayFlag == "P") {
					KIND kind = symbolTable[pos.first][pos.second]->kind;
					if (kind != KIND::POINTER) {
						illegal_use();
					}
					string expAddr = arraynode->attributes["expAddr"];
					/*for (auto x : arraynode->code)ret->code.push_back(x);
					ret->code.push_back(new Quadruples("=[]", tmp, expAddr, createTemp()));*/
					for (auto x : arraynode->code)ret->code.insert(prev(ret->code.end()), x);
					ret->code.back()->op += "[]";
					ret->code.back()->opnd2 = expAddr;
					if (fieldFlag == "") {
						string type = symbolTable[pos.first][pos.second]->type.substr(1);
						if (isdigit(type[0])) {
							for (int i = 1; i < type.length(); i++) {
								if (!isdigit(type[i])) {
									type = type.substr(i);
									break;
								}
							}
						}
						tempType[cur_function][tmpIdx] = type;
						ret->attributes["type"] = type;
					} else {
						string type = symbolTable[pos.first][pos.second]->type;
						for (int i = 1; i < type.length(); i++) {
							if (!isdigit(type[i])) {
								type = type.substr(i); break;
							}
						}
						pos = lookup(type);
						tempType[cur_function][tmpIdx] = type;
						int sub_scope = symbolTable[pos.first][pos.second]->info;
						auto fieldPos = find_if(symbolTable[sub_scope].begin(), symbolTable[sub_scope].end(), [=](Symbol* s) {
							return s->name == fieldFlag;
						});
						if (fieldPos == symbolTable[sub_scope].end()) {
							non_definition(fieldFlag);
						}
						type = (*fieldPos)->type;
						ret->attributes["type"] = type;
						//ret->code.back()->op += "." + fieldFlag;
						ret->code.push_back(new Quadruples("=." + fieldFlag, tmp, "", createTemp()));
						tmp = getTemp();
						tmpIdx = stoi(tmp.substr(1));
						tempType[cur_function][tmpIdx] = type;
						ret->attributes["addr"] = tmp;
					}
				} else {
					if (fieldFlag == "") {
						KIND kind = symbolTable[pos.first][pos.second]->kind;
						if (kind != KIND::VARIABLE && kind != KIND::POINTER) {
							illegal_use();
						}
						string type = symbolTable[pos.first][pos.second]->type;
						ret->attributes["type"] = symbolTable[pos.first][pos.second]->type;
						tempType[cur_function][tmpIdx] = type;
					} else {
						string type = symbolTable[pos.first][pos.second]->type;
						pos = lookup(type);
						int sub_scope = symbolTable[pos.first][pos.second]->info;
						auto fieldPos = find_if(symbolTable[sub_scope].begin(), symbolTable[sub_scope].end(), [=](Symbol* s) {
							return s->name == fieldFlag;
						});
						if (fieldPos == symbolTable[sub_scope].end()) {
							non_definition(fieldFlag);
						}
						type = (*fieldPos)->type;
						ret->attributes["type"] = type;
						ret->code.back()->op += "." + fieldFlag;
						tempType[cur_function][tmpIdx] = type;
					}
				}
			} else {
				//变量是左值，用于赋值
				ret->attributes["addr"] = get_entry(pos);
				if (arrayFlag == "P") {
					KIND kind = symbolTable[pos.first][pos.second]->kind;
					if (kind != KIND::POINTER) {
						illegal_use();
					}
					ret->attributes["expAddr"] = arraynode->attributes["expAddr"];
					ret->code = arraynode->code;
					if (fieldFlag == "") {
						string type = symbolTable[pos.first][pos.second]->type.substr(1);
						if (isdigit(type[0])) {
							for (int i = 1; i < type.length(); i++) {
								if (!isdigit(type[i])) {
									type = type.substr(i); break;
								}
							}
						}
						ret->attributes["type"] = type;
					} else {
						string type = symbolTable[pos.first][pos.second]->type;
						type.erase(0, 1);
						for (int i = 0; i < type.length(); i++) {
							if (!isdigit(type[i])) {
								type = type.substr(i);
								break;
							}
						}
						pos = lookup(type);
						int subScope = symbolTable[pos.first][pos.second]->info;
						auto fieldPos = find_if(symbolTable[subScope].begin(), symbolTable[subScope].end(), [=](Symbol* s) {
							return s->name == fieldFlag;
						});
						if (fieldPos == symbolTable[subScope].end()) {
							non_definition(fieldFlag);
						}
						ret->attributes["type"] = (*fieldPos)->type;
					}
				} else {
					//没有出现[]
					KIND kind = symbolTable[pos.first][pos.second]->kind;
					if (kind != KIND::VARIABLE && kind != KIND::POINTER) {
						illegal_use();
					}
					if (!symbolTable[pos.first][pos.second]->updatable) {
						unupdatable(id);
					}
					if (fieldFlag == "") {
						string type = symbolTable[pos.first][pos.second]->type;
						/*if (isdigit(type[0])) {
							for (int i = 1; i < type.length(); i++) {
								if (!isdigit(type[i])) {
									type = type.substr(i);
									break;
								}
							}
						}*/
						ret->attributes["type"] = type;
					} else {
						string type = symbolTable[pos.first][pos.second]->type;
						pos = lookup(type);
						int sub_scope = symbolTable[pos.first][pos.second]->info;
						auto fieldPos = find_if(symbolTable[sub_scope].begin(), symbolTable[sub_scope].end(), [=](Symbol* s) {
							return s->name == fieldFlag;
						});
						if (fieldPos == symbolTable[sub_scope].end()) {
							non_definition(fieldFlag);
						}
						ret->attributes["type"] = (*fieldPos)->type;
					}
				}
			}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* conditional_expression();
	Sign* is_array() {
		auto ret = new Sign("is-array");
		if (cur_sign == "[") {
			match("[");
			auto node = conditional_expression();
			ret->child.push_back(node);
			ret->code = node->code;
			string type = node->attributes["type"];
			if (type != "i") {
				invalid_conversion("int", type);
			}
			match("]");
			ret->attributes["arrayFlag"] = "P";
			ret->attributes["expAddr"] = node->attributes["addr"];
		} else if (in_first("", "is-array") && in_follow(cur_sign, "is-array")) {
			ret->attributes["arrayFlag"] = "";
		} else {
			//output_follow_set("ID");
			illegal_use();
		}
		return ret;
	}
	Sign* is_field() {
		auto ret = new Sign("is-field");
		if (cur_sign == ".") {
			match(".");
			auto node = ID();
			ret->child.push_back(node);
			string field = node->attributes["id"];
			ret->attributes["fieldFlag"] = field;
		} else if (in_first("", "is-field") && in_follow(cur_sign, "is-field")) {
			ret->attributes["fieldFlag"] = "";
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* ass_operator() {
		auto ret = new Sign("ass-operator");
		if (cur_sign == "=") {
			match("=");
			ret->attributes["ass-operator"] = "=";
		} else if (cur_sign == "+=") {
			match("+=");
			ret->attributes["ass-operator"] = "+=";
		} else if (cur_sign == "-=") {
			match("-=");
			ret->attributes["ass-operator"] = "-=";
		} else if (cur_sign == "*=") {
			match("*=");
			ret->attributes["ass-operator"] = "*=";
		} else if (cur_sign == "/=") {
			match("/=");
			ret->attributes["ass-operator"] = "/=";
		} else if (cur_sign == "%=") {
			match("%=");
			ret->attributes["ass-operator"] = "%=";
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* additive_expression();
	Sign* extra_additive_expression();
	Sign* conditional_expression() {
		auto ret = new Sign("conditional-expression");
		if (in_first(cur_sign, 2, "additive-expression", "extra-additive-expression")) {
			auto left = additive_expression();
			ret->child.push_back(left);
			string left_type = left->attributes["type"];
			auto right = extra_additive_expression();
			ret->child.push_back(right);
			string right_type = right->attributes["type"];
			if (left_type == "i" && right_type == "i" || right_type == "") {
				ret->attributes["type"] = left_type;
			} else {
				invalid_conversion(left_type, right_type);
			}
			auto rel_operator = right->attributes["rel-operator"];
			if (rel_operator != "") {
				string tmp = createTemp();
				int tmpIdx = tempCnt;
				ret->code = left->code;
				for (auto x : right->code)ret->code.push_back(x);
				ret->code.push_back(
					new Quadruples(
						rel_operator, left->attributes["addr"], right->attributes["addr"], tmp
					)
				);
				ret->attributes["addr"] = tmp;
				tempType[cur_function][tmpIdx] = left_type;
			} else {
				ret->code = left->code;
				ret->attributes["addr"] = left->attributes["addr"];
			}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* relation_operator();
	Sign* additive_expression();
	Sign* extra_additive_expression() {
		auto ret = new Sign("extra-additive-expression");
		if (in_first(cur_sign, 2, "relation-operator", "additive-expression")) {
			auto left = relation_operator();
			ret->child.push_back(left);
			auto right = additive_expression();
			ret->child.push_back(right);
			string type = right->attributes["type"];
			if (type != "i") {
				invalid_conversion("int", type);
			}
			ret->attributes["type"] = type;
			ret->attributes["rel-operator"] = left->attributes["rel-operator"];
			ret->attributes["addr"] = right->attributes["addr"];
			ret->code = right->code;
		} else if (in_first("", "extra-additive-expression") && in_follow(cur_sign, "extra-additive-expression")) {
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* relation_operator() {
		auto ret = new Sign("relation-operator");
		if (cur_sign == "<=") {
			match("<=");
			ret->attributes["rel-operator"] = "<=";
		} else if (cur_sign == "<") {
			match("<");
			ret->attributes["rel-operator"] = "<";
		} else if (cur_sign == ">") {
			match(">");
			ret->attributes["rel-operator"] = ">";
		} else if (cur_sign == ">=") {
			match(">=");
			ret->attributes["rel-operator"] = ">=";
		} else if (cur_sign == "==") {
			match("==");
			ret->attributes["rel-operator"] = "==";
		} else if (cur_sign == "!=") {
			match("!=");
			ret->attributes["rel-operator"] = "!=";
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* add_operator();
	Sign* multiplicative_expression();
	Sign* additive_expression() {
		auto ret = new Sign("additive-expression");
		if (in_first(cur_sign, "multiplicative-expression")) {
			auto left = multiplicative_expression();
			ret->child.push_back(left);
			ret->attributes["type"] = left->attributes["type"];
			ret->code = left->code;
			ret->attributes["addr"] = left->attributes["addr"];
			while (cur_sign == "+" || cur_sign == "-") {
				string op = cur_sign;
				ret->child.push_back(add_operator());
				auto right = multiplicative_expression();
				ret->child.push_back(right);
				string left_type = ret->attributes["type"];
				string right_type = right->attributes["type"];
				if (left_type == "i" || right_type == "i") {
					ret->attributes["type"] = left_type;
				} else {
					invalid_conversion(left_type, right_type);
				}
				for (auto x : right->code)ret->code.push_back(x);
				string tmp = createTemp();
				int tmpIdx = tempCnt;
				ret->code.push_back(new Quadruples(op, ret->attributes["addr"], right->attributes["addr"], tmp));
				ret->attributes["addr"] = tmp;
				tempType[cur_function][tmpIdx] = "i";
			}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* add_operator() {
		auto ret = new Sign("add-operator");
		if (cur_sign == "+") {
			match("+");
			ret->attributes["add-operator"] = "+";
		} else if (cur_sign == "-") {
			match("-");
			ret->attributes["add-operator"] = "-";
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* mul_operator();
	Sign* primary_expression();
	Sign* multiplicative_expression() {
		auto ret = new Sign("multiplicative-expression");
		if (in_first(cur_sign, 2, "primary-expression", "extra-primary-expression")) {
			auto left = primary_expression();
			ret->child.push_back(left);
			ret->attributes["type"] = left->attributes["type"];
			ret->code = left->code;
			ret->attributes["addr"] = left->attributes["addr"];
			while (in_first(cur_sign, "mul-operator")) {
				string op = cur_sign;
				ret->child.push_back(mul_operator());
				auto right = primary_expression();
				ret->child.push_back(right);
				string left_type = ret->attributes["type"];
				string right_type = right->attributes["type"];
				if (left_type == "i" || right_type == "i") {
					ret->attributes["type"] = left_type;
				} else {
					invalid_conversion(left_type, right_type);
				}
				for (auto x : right->code)ret->code.push_back(x);
				string tmp = createTemp();
				int tmpIdx = tempCnt;
				ret->code.push_back(new Quadruples(op, ret->attributes["addr"], right->attributes["addr"], tmp));
				ret->attributes["addr"] = tmp;
				tempType[cur_function][tmpIdx] = "i";
			}
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* mul_operator() {
		auto ret = new Sign("mul-operator");
		if (cur_sign == "*") {
			match("*");
			ret->attributes["mul-operator"] = "*";
		} else if (cur_sign == "/") {
			match("/");
			ret->attributes["mul-operator"] = "/";
		} else if (cur_sign == "%") {
			match("%");
			ret->attributes["mul-operator"] = "%";
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* call_function();
	Sign* primary_expression() {
		auto ret = new Sign("primary-expression");
		//ID(...)应该被识别为函数调用(call-function)，因此要向前方多看一个符号，如果不是左括号则认为是简单变量
		if (in_first(cur_sign, "variable") && get_next_sign() != "(") {
			//E->ID
			auto node = variable(0);
			ret->child.push_back(node);
			ret->attributes["type"] = node->attributes["type"];
			ret->attributes["addr"] = node->attributes["addr"];
			ret->code = node->code;
		} else if (cur_sign == "NUM") {
			//E->NUM
			auto node = NUM();
			ret->child.push_back(node);
			ret->attributes["type"] = node->attributes["type"];
			ret->attributes["addr"] = node->attributes["num"];
		} else if (cur_sign == "(") {
			//E->(E1)
			match("(");
			auto node = expression();
			ret->child.push_back(node);
			string type = node->attributes["type"];
			ret->attributes["type"] = type;
			ret->attributes["addr"] = node->attributes["addr"];
			ret->code = node->code;
			match(")");
		} else if (in_first(cur_sign, "call-function") && get_next_sign() == "(") {
			//E->ID()
			auto node = call_function();
			ret->child.push_back(node);
			ret->attributes["type"] = node->attributes["type"];
			ret->code = node->code;
			string function_name = node->attributes["id"];
			if (ret->attributes["type"] == "v") {
				ret->code.push_back(new Quadruples("call", function_name, to_string(parLen[function_name]), ""));
			} else {
				string tmp = createTemp();
				int tmpIdx = tempCnt;
				ret->code.push_back(
					new Quadruples(
						"call", function_name, to_string(parLen[function_name]), tmp
					)
				);
				ret->attributes["addr"] = tmp;
				tempType[cur_function][tmpIdx] = node->attributes["type"];
			}
		} else if (cur_sign == "-") {
			//E->-E1
			match("-");
			auto node = primary_expression();
			ret->child.push_back(node);
			string type = node->attributes["type"];
			ret->attributes["type"] = type;
			ret->code = node->code;
			ret->code.push_back(new Quadruples("-", "0", node->attributes["addr"], createTemp()));//t=0-E1.addr
			ret->attributes["addr"] = getTemp();//E.addr=t
			tempType[cur_function][stoi(getTemp().substr(1))] = type;
			//ret->code.push_back(new Quadruples("=", createTemp(), "", ret->attributes["addr"]));
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* arguments();
	Sign* call_function() {
		auto ret = new Sign("call-function");
		if (cur_sign == "ID") {
			auto node = ID();
			ret->child.push_back(node);
			string id = node->attributes["id"];
			match("(");
			node = arguments();
			ret->child.push_back(node);
			for (auto x : node->code)ret->code.push_back(x);
			match(")");
			attachTypes(id);
			auto pos = lookup(id);
			if (pos.first != -1) {
				ret->attributes["type"] = symbolTable[pos.first][pos.second]->type;
			} else {
				non_definition(id);
			}
			ret->attributes["id"] = id;
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* argument_list();
	Sign* arguments() {
		auto ret = new Sign("arguments");
		if (in_first(cur_sign, "argument-list")) {
			auto node = argument_list();
			ret->child.push_back(node);
			ret->code = node->code;
		} else if (in_first("", "arguments") && in_follow(cur_sign, "arguments")) {
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* extra_argument_list();
	Sign* argument_list() {
		auto ret = new Sign("argument-list");
		if (in_first(cur_sign, 2, "expression", "extra-argument-list")) {
			auto left = expression();
			ret->child.push_back(left);
			string type = left->attributes["type"];
			if (type[0] != 'P' && type[0]!='i') {
				type.insert(0, to_string(type.length()));
			}/* else {
				if (type != "i"&&type != "v")type.insert(0, to_string(type.length()));
			}*/
			par_types.push_back(type);
			auto right = extra_argument_list();
			ret->child.push_back(right);
			for (auto x : right->code)ret->code.push_back(x);//A.code+=eA.code
			for (auto x : left->code)ret->code.push_back(x);//A.code+=E.code
			ret->code.push_back(new Quadruples("par", "", "", left->attributes["addr"]));//A.code+=(par,E.addr, $, $)
		} else if (in_first("", "argument-list") && in_follow(cur_sign, "argument-list")) {
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	Sign* extra_argument_list() {
		auto ret = new Sign("extra-argument-list");
		if (cur_sign == ",") {
			match(",");
			auto left = expression();
			ret->child.push_back(left);
			string type = left->attributes["type"];
			if (type[0] != 'P' && type[0] != 'i') {
				type.insert(0, to_string(type.length()));
			}
			par_types.push_back(type);
			//        puts("压入一个参数");
			auto right = extra_argument_list();
			ret->child.push_back(right);
			for (auto x : right->code)ret->code.push_back(x);
			for (auto x : left->code)ret->code.push_back(x);
			ret->code.push_back(new Quadruples("par", "", "", left->attributes["addr"]));//A.code+=(par, $, $, E.addr)
		} else if (in_first("", "extra-argument-list") && in_follow(cur_sign, "extra-argument-list")) {
			//        do nothing
		} else {
			illegal_use();
		}
		return ret;
	}
	void endParser(const char*PI) {
		remove(PI);
	}
	void parser(const char *PI) {
		initiation("productions.dat"/*, "intermediate code.txt"*/);
		parse(PI);
		endParser(PI);
	}

}


#endif // PARSER_H_INCLUDED
