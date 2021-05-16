#pragma once
#ifndef SCANNER_H_INCLUDED
#define SCANNER_H_INCLUDED


#include <bits/stdc++.h>
#define rg_ne(i, a, b, s) for(auto i = a; i != b; i += s)
#define rg(i, a, b, s) for(auto i = a; (b > a)?(i < b):(i > b); i += s)
#define ERROR -1 // ��������ʶ��ĵ��ʣ����ִʷ�����
#define START 0 // ��ʼ״̬��û�ж����κε���
#define LETTER 1 // ʶ����ĸ���״̬
#define CONSTANT 2 // ʶ�������״̬
#define OPERATOR 3 // ʶ���������+ - *���״̬
#define SEPARATOR 4 // ʶ�𵽷ָ�����( ; { ) }���״̬
#define VAR 5 // ʶ�𵽱������״̬
#define KEYWORD 6 // ʶ�𵽹ؼ�����if, else, int, void���״̬
#define COMMENT_IN 7 // ʶ��/����Ϊ����Ĳ��ֿ�����ע��
#define COMMENT_OUT 8 // ��ע����ʶ��*������ķ��ſ�����/�������Ϊ������*/����ע��
#define COMMENT 9 // ��COMMENT_IN״̬��ʶ��*���õ������ĵ��ʷ���/*����Ϊ����Ĳ�����ע��
#define END 10 // ʶ����һ���Ϸ����ʵ�״̬
#define OPERATOR_END 11 // ����+=, -=, *=, /=��ʶ���ڴ�״̬������ʶ���κ��ַ���תΪEND
#define COMMENT_END 12 // ����һ��ע�͵�ʶ��
#define STATE_NUM 13 // �ܵ�״̬��,ERROR����
using namespace std;
namespace scanner {

	unordered_map<string, int> token_cnts;
	unordered_set<string> Keyword;
	unordered_set<char>Operator, Separator;
	unordered_set<string> output_keywords, output_identifiers;
	string to_string(string _s) {
		return(_s);
	}
	template<class A, class B>
	string to_string(const pair<A, B>& p) {
		return("<" + to_string(p.first) + ", " + to_string(p.second) + ">");
	}
	FILE* fin, *fout;
	char curChar;
	int curState = START;
	string curToken = "";
	queue<string> synthesized_attribute;
	void concat() {
		curToken.push_back(curChar);
	}
	void getChar() {
		curChar = fgetc(fin);
	}
	void retract() {
		fseek(fin, -1, SEEK_CUR);
	}
	void skipBlanks() {
		do {
			curChar = fgetc(fin);
		} while (curChar == ' ' || curChar == '\t' || curChar == '\n');
		retract();
	}
	int stateMatrix[STATE_NUM][127];
	void initiation() {
		Separator.insert('(');
		Separator.insert(')');
		Separator.insert(',');
		Separator.insert(';');
		Separator.insert(':');
		Separator.insert('[');
		Separator.insert(']');
		Separator.insert('{');
		Separator.insert('}');
		Separator.insert('\n');
		Separator.insert(' ');
		Separator.insert('\t');
		Separator.insert('.');

		Operator.insert('+');
		Operator.insert('-');
		Operator.insert('*');
		Operator.insert('/');
		Operator.insert('<');
		Operator.insert('>');
		Operator.insert('!');
		Operator.insert('=');
		Operator.insert('%');

		Keyword.insert("else");
		Keyword.insert("if");
		Keyword.insert("switch");
		Keyword.insert("case");
		Keyword.insert("default");
		Keyword.insert("int");
		Keyword.insert("void");
		Keyword.insert("struct");
		Keyword.insert("return");
		Keyword.insert("break");
		Keyword.insert("continue");
		Keyword.insert("goto");
		Keyword.insert("while");
		Keyword.insert("for");
		memset(stateMatrix, ERROR, sizeof(stateMatrix));
		rg(i, 'A', 'Z' + 1, 1) {
			//        START״̬��ʶ���κ���ĸ��״̬ת��ΪLETTER
			stateMatrix[START][i] = LETTER;
			stateMatrix[START][i + 32] = LETTER;
			//        LETTER״̬��ʶ���κ���ĸ״̬����
			stateMatrix[LETTER][i] = LETTER;
			stateMatrix[LETTER][i + 32] = LETTER;
			//        VARIABLE״̬��ʶ���κ���ĸ״̬����
			stateMatrix[VAR][i] = VAR;
			stateMatrix[VAR][i + 32] = VAR;
			//        OPERATOR״̬��ʶ���κ���ĸ�������������ʶ��
			stateMatrix[OPERATOR][i] = END;
			stateMatrix[OPERATOR][i + 32] = END;
		}
		rg(i, '0', '9' + 1, 1) {
			//        START״̬��ʶ�����֣�״̬ת��ΪCONSTANT
			stateMatrix[START][i] = CONSTANT;
			//        LETTER״̬��ʶ�����֣�˵��������ʲ������ǹؼ��֣�ֻ�����Ǳ�������״̬תΪVARIABLE
			stateMatrix[LETTER][i] = VAR;
			//        VARIABLE״̬��ʶ���κ�����״̬����
			stateMatrix[VAR][i] = VAR;
			//        CONSTANT״̬��ʶ���κ�����״̬����
			stateMatrix[CONSTANT][i] = CONSTANT;
			//        OPERATOR״̬��ʶ���κ����֣������������ʶ��
			stateMatrix[OPERATOR][i] = END;
		}

		for (char c : Operator) {
			//        START״̬��ʶ���κ��������״̬ת��ΪOPERATOR
			stateMatrix[START][c] = OPERATOR;
			//        LETTER״̬��ʶ���κ����������������ʶ����Ϊ�ؼ��ֻ�������ж������ܳ��������������ʱ���ܲ�֪��ʶ�𵽵��ǹؼ��ֻ��Ǳ�����
			stateMatrix[LETTER][c] = END;
			//        VARIABLE״̬��ʶ���������������������ʶ��
			stateMatrix[VAR][c] = END;
			//        CONSTANT״̬��ʶ����������������ֵ�ʶ��
			stateMatrix[CONSTANT][c] = END;
		}
		//    OPERATOR״̬�¿���ʶ��=����+=��-=��*=��/=��==��!=��״̬ת��ΪOPERATOR_END��������ʶ���κε��ʶ�ת��ΪEND
		stateMatrix[OPERATOR]['='] = OPERATOR_END;
		for (auto c : Separator) {
			//        �ڳ�ʼ״̬��ʶ���κηָ�����ת��ΪSEPARATOR
			stateMatrix[START][c] = SEPARATOR;
			//        ������״̬��ʶ�𵽷ָ�������Ϊ��ǰ����ʶ�����
			stateMatrix[LETTER][c] = END;
			stateMatrix[VAR][c] = END;
			stateMatrix[CONSTANT][c] = END;
			stateMatrix[OPERATOR][c] = END;
		}
		rg(i, 0, STATE_NUM, 1) {
			stateMatrix[i]['#'] = END;
		}
		stateMatrix[START]['/'] = COMMENT_IN;
		//stateMatrix[COMMENT_IN][' '] = END;
		fill(begin(stateMatrix[COMMENT_IN]), end(stateMatrix[COMMENT_IN]), END);
		stateMatrix[COMMENT_IN]['*'] = COMMENT;
		stateMatrix[COMMENT_IN]['='] = OPERATOR_END;
		//    ��OPERATOR_END��SEPARATOR״̬��ʶ���κ��ַ�����Ϊ��ǰ����ʶ�����
		fill(begin(stateMatrix[SEPARATOR]), end(stateMatrix[SEPARATOR]), END);
		fill(begin(stateMatrix[OPERATOR_END]), end(stateMatrix[OPERATOR_END]), END);

		//    ��ע����ʶ�������*�ַ���Ȼ��ע��
		//    ��ע����ʶ��*[^/]�ַ�����Ȼ��ע��
		//    ��ע����ʶ��*/����ע��

		rg(i, 0, 128, 1) {
			stateMatrix[COMMENT][i] = COMMENT;
			stateMatrix[COMMENT_OUT][i] = COMMENT;
			stateMatrix[COMMENT_END][i] = END;
		}
		stateMatrix[COMMENT]['*'] = COMMENT_OUT;
		stateMatrix[COMMENT_OUT]['/'] = COMMENT_END;

		stateMatrix[START]['#'] = ERROR;

	}
	bool is_digit(string n) {
		for (auto c : n) {
			if (!isdigit(c)) {
				return 0;
			}
		}
		return 1;
	}
	bool is_keyword(string n) {
		return Keyword.find(n) != Keyword.end();
	}
	bool is_separator(string n) {
		return Separator.find(n[0]) != Separator.end();
	}
	bool is_operator(string n) {
		return Operator.find(n[0]) != Operator.end();
	}
	bool scan(const char *input_file_name, const char *output_file_name) {
		fin = fopen(input_file_name, "r");
		if (!fin) {
			printf("'%s' in not found.", input_file_name);
			exit(0);
		}
		fout = fopen(output_file_name, "w");
		fprintf(fout, "#");
		fseek(fin, -1, SEEK_END);
		//��ȡ�����ļ����һ���ַ�
		getChar();
		//�������#����ĩβ����һ��#
		if (curChar != '#') {
			FILE* fp = fopen(input_file_name, "a");
			fprintf(fp, "#");
			fclose(fp);
		}
		fseek(fin, 0, SEEK_SET);
		//    fclose(fin);
		//    fin = fopen(input_file_name, "r");
		//    if(!fin){
		//        puts("The file does not exist.\n");
		//        return 0;
		//    }
		curState = START;
		//�����ļ���ͷ�Ŀհ�
		skipBlanks();
		getChar();
		while (stateMatrix[curState][curChar] != ERROR) {
			concat();
			if (stateMatrix[curState][curChar] == END) {
				curToken.pop_back();
				retract();
				if (curState == COMMENT_END) {

				} else if (is_keyword(curToken)) {
					curState = KEYWORD;
				} else if (is_digit(curToken)) {
					curState = CONSTANT;
				} else if (is_separator(curToken)) {
					curState = SEPARATOR;
				} else if (is_operator(curToken)) {
					curState = OPERATOR;
				} else {
					curState = VAR;
				}
				switch (curState) {
				case KEYWORD: {
					fprintf(fout, " %s", curToken.c_str());
					token_cnts["keyword"]++;
					//printf("keyword ");
					break;
				}
				case VAR: {
					fprintf(fout, " %s", "ID");
					token_cnts["ID"]++;
					//printf("ID ");
					synthesized_attribute.push(curToken);
					break;
				}
				case CONSTANT: {
					fprintf(fout, " %s", "NUM");
					token_cnts["NUM"]++;
					//printf("NUM ");
					synthesized_attribute.push(curToken);
					break;
				}
				case SEPARATOR: {
					fprintf(fout, " %s", curToken.c_str());
					token_cnts["separator"]++;
					//printf("separator ");
					break;
				}
				case OPERATOR: {
					fprintf(fout, " %s", curToken.c_str());
					token_cnts["operator"]++;
					//printf("operator ");
					break;
				}
				case COMMENT_END: {
					//printf("comment ");
					break;
				}
				}
				//printf("identified: %s\n", curToken.c_str());
				curToken.clear();
				curState = START;
				skipBlanks();
			} else {
				curState = stateMatrix[curState][curChar];
			}
			getChar();
		}
		fclose(fin);
		fprintf(fout, " #");
		fclose(fout);
		if (curState == START && curChar == '#') {
			//���һ��������#����״̬ΪSTARTʱʶ��ɹ�������true
			queue<string> t = synthesized_attribute;
			//fout = fopen("synthesized attribute.txt", "w");
			while (!synthesized_attribute.empty()) {
				string attr = synthesized_attribute.front();
				//fprintf(fout, "%s\n", attr.c_str());
				synthesized_attribute.pop();
			}
			synthesized_attribute = t;
			//fclose(fout);
			return 1;
		} else {
			return 0;
		}
	}
	string scanner(const char *filename) {
		string outputFileName = "SO.txt";
		initiation();
		if (!scan(filename, outputFileName.c_str())) {
			puts("Lexical Error.\a");
			printf("Last State: %d, Last Char: %c, Last token: %s", curState, curChar, curToken.c_str());
			exit(0);
		} else {
			//puts("Lexeme analysis successful.");
		}
		return outputFileName;
	}

}


#endif // SCANNER_H_INCLUDED
