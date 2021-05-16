#pragma once
#include"quadruples.h"
using namespace std;
using namespace quadruple_space;
namespace machine_independent_optimizer {
	vector<vector<Quadruples*> > parseBlock(vector<Quadruples*> functionBlock) {
		vector<vector<Quadruples*> > ret;
		vector<Quadruples*> lastBlock;
		bool flag = 1;
		for (Quadruples* q : functionBlock) {
			if (q->op[0] == 'l') {
				if (!lastBlock.empty()) {
					ret.push_back(lastBlock);
					lastBlock.clear();
				}
				lastBlock.push_back(q);
				flag = 1;
			} else {
				if (!flag)continue;
				if (q->op == "ret" || q->op == "jmp") {
					lastBlock.push_back(q);
					ret.push_back(lastBlock);
					lastBlock.clear();
					flag = 0;
				} else {
					lastBlock.push_back(q);
				}
			}
		}
		if (!lastBlock.empty())ret.push_back(lastBlock);
		return ret;
	}
	void optimizer() {
		quadruple_space::initiation();
		vector<Quadruples*> codeList;
		vector<Quadruples*> functionBlock;
		while (1) {
			auto curInstr = getNextQuadruple();
			if (!curInstr)break;
			if (curInstr->op == "struct") {
				codeList.push_back(curInstr);
				do {
					curInstr = getNextQuadruple();
					codeList.push_back(curInstr);
				} while (curInstr->op != "ends");
			}
			if (curInstr->op == "proc") {
				codeList.push_back(curInstr);
				do {
					curInstr = getNextQuadruple();
					if (curInstr->op == "endp") {
						for (auto b : parseBlock(functionBlock)) {
							for (Quadruples* q : b) {
								codeList.push_back(q);
							}
						}
						codeList.push_back(curInstr);
						functionBlock.clear();
						break;
					}
					functionBlock.push_back(curInstr);
				} while (1);
			}
		}
		swap(codeList, threeAddrCodes);
	}
}