#pragma once
#include<bits/stdc++.h>
using namespace std;
namespace quadruple_space {
	struct Quadruples {
		string op;
		string opnd1;
		string opnd2;
		string result;
		Quadruples(string op, string opnd1, string opnd2, string result) {
			this->op = op;
			this->opnd1 = opnd1;
			this->opnd2 = opnd2;
			this->result = result;
		}
		string toString() {
			string ret = op;
			ret += " " + (opnd1 == "" ? "$" : opnd1);
			ret += " " + (opnd2 == "" ? "$" : opnd2);
			ret += " " + (result == "" ? "$" : result);
			return ret;
		}
	};
	vector<Quadruples*> threeAddrCodes;
	auto curIt = threeAddrCodes.begin();
	void initiation() {
		curIt = threeAddrCodes.begin();
	}
	Quadruples* getNextQuadruple() {
		if (curIt == threeAddrCodes.end())return 0;
		auto ret = *curIt;
		curIt++;
		return ret;
	}
}