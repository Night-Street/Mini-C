#pragma once
#ifndef SCANNER_H_INCLUDED
#define SCANNER_H_INCLUDED


#include <bits/stdc++.h>
#define rg_ne(i, a, b, s) for(auto i = a; i != b; i += s)
#define rg(i, a, b, s) for(auto i = a; (b > a)?(i < b):(i > b); i += s)
#define ERROR -1 // 遇到不可识别的单词，出现词法错误
#define START 0 // 初始状态，没有读入任何单词
#define LETTER 1 // 识别字母后的状态
#define CONSTANT 2 // 识别常数后的状态
#define OPERATOR 3 // 识别运算符如+ - *后的状态
#define SEPARATOR 4 // 识别到分隔符如( ; { ) }后的状态
#define VAR 5 // 识别到变量后的状态
#define KEYWORD 6 // 识别到关键字如if, else, int, void后的状态
#define COMMENT_IN 7 // 识别到/，认为后面的部分可能是注释
#define COMMENT_OUT 8 // 从注释中识别到*，后面的符号可能是/，因此认为可能由*/结束注释
#define COMMENT 9 // 在COMMENT_IN状态下识别到*，得到连续的单词符号/*，认为后面的部分是注释
#define END 10 // 识别完一个合法单词的状态
#define OPERATOR_END 11 // 结束+=, -=, *=, /=的识别，在此状态下往后识别任何字符都转为END
#define COMMENT_END 12 // 结束一个注释的识别
#define STATE_NUM 13 // 总的状态数,ERROR除外
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
			//        START状态下识别任何字母，状态转变为LETTER
			stateMatrix[START][i] = LETTER;
			stateMatrix[START][i + 32] = LETTER;
			//        LETTER状态下识别任何字母状态不变
			stateMatrix[LETTER][i] = LETTER;
			stateMatrix[LETTER][i + 32] = LETTER;
			//        VARIABLE状态下识别任何字母状态不变
			stateMatrix[VAR][i] = VAR;
			stateMatrix[VAR][i + 32] = VAR;
			//        OPERATOR状态下识别任何字母，结束运算符的识别
			stateMatrix[OPERATOR][i] = END;
			stateMatrix[OPERATOR][i + 32] = END;
		}
		rg(i, '0', '9' + 1, 1) {
			//        START状态下识别数字，状态转变为CONSTANT
			stateMatrix[START][i] = CONSTANT;
			//        LETTER状态下识别到数字，说明这个单词不可能是关键字，只可能是变量名，状态转为VARIABLE
			stateMatrix[LETTER][i] = VAR;
			//        VARIABLE状态下识别任何数字状态不变
			stateMatrix[VAR][i] = VAR;
			//        CONSTANT状态下识别任何数字状态不变
			stateMatrix[CONSTANT][i] = CONSTANT;
			//        OPERATOR状态下识别任何数字，结束运算符的识别
			stateMatrix[OPERATOR][i] = END;
		}

		for (char c : Operator) {
			//        START状态下识别任何运算符，状态转变为OPERATOR
			stateMatrix[START][c] = OPERATOR;
			//        LETTER状态下识别任何运算符，结束单词识别，因为关键字或变量名中都不可能出现运算符，但此时可能不知道识别到的是关键字还是变量名
			stateMatrix[LETTER][c] = END;
			//        VARIABLE状态下识别到运算符，结束变量名的识别
			stateMatrix[VAR][c] = END;
			//        CONSTANT状态下识别到运算符，结束数字的识别
			stateMatrix[CONSTANT][c] = END;
		}
		//    OPERATOR状态下可以识别到=，即+=，-=，*=，/=，==，!=，状态转变为OPERATOR_END，再往后识别任何单词都转变为END
		stateMatrix[OPERATOR]['='] = OPERATOR_END;
		for (auto c : Separator) {
			//        在初始状态下识别任何分隔符，转变为SEPARATOR
			stateMatrix[START][c] = SEPARATOR;
			//        在其他状态下识别到分隔符则认为当前单词识别完毕
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
		//    在OPERATOR_END和SEPARATOR状态下识别到任何字符都认为当前单词识别完毕
		fill(begin(stateMatrix[SEPARATOR]), end(stateMatrix[SEPARATOR]), END);
		fill(begin(stateMatrix[OPERATOR_END]), end(stateMatrix[OPERATOR_END]), END);

		//    在注释中识别任意非*字符依然是注释
		//    在注释中识别*[^/]字符串依然是注释
		//    在注释中识别*/结束注释

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
		//读取输入文件最后一个字符
		getChar();
		//如果不是#，在末尾加上一个#
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
		//跳过文件开头的空白
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
			//最后一个符号是#，且状态为START时识别成功，返回true
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
