#include "parser.h"
#include "scanner.h"
#include "object_code_generator.h"
#include "machine-independent optimizer.h"
int main(int argc, char **argv) {

	string SI = "";
	if (argc >= 2) {
		SI = argv[1];
	} else {
		//词法分析器
		//SI = "scanner-input.txt";
		return 0;
	}
	string SO = scanner::scanner(SI.c_str());
	//语法分析器、语义分析器和中间代码生成器
	parser::parser(SO.c_str());
	//for (auto p1 : tempType) {
	//	cout << p1.first << ":" << endl;
	//	for (auto p2 : p1.second) {
	//		cout << "\t" << p2.first << ": " << p2.second << endl;
	//	}
	//}
	//中间代码优化器
	machine_independent_optimizer::optimizer();
	//目标代码生成器
	string OC = object_code_generator::objectCodeGenerator(SI.c_str());
	//用MASM汇编程序编译目标代码，生成可执行的文件
	system(("bin\\ml -c -coff " + OC).c_str());//MASM编译操作命令
	remove(OC.c_str());//删除.asm文件
	object_code_generator::updateFileExtension(OC, "obj");//后缀名改为.obj
	system(("bin\\link32 /subsystem:console " + OC).c_str());//MASM链接操作命令
	remove(OC.c_str());//删除.obj文件
	//object_code_generator::updateFileExtension(OC, "exe");//后缀名改为.exe
	//system(OC.c_str());//运行.exe程序
	return 0;
}
