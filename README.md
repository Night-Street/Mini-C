# Mini-C
使用说明：
1、解压缩MASM.rar。
2、进入Debug目录，将Mini-C.exe复制到MASM目录下。
3、进入Mini-C目录，将productions.dat复制到MASM目录下。
4、在MASM目录下打开命令行窗口，执行Mini-C.exe xxx.txt。其中xxx.txt是源代码文件，扩展名为txt，cpp或dat均可。执行完毕后会得到一个与源文件同名的可执行文件xxx.exe。
5、运行xxx.exe。



MASM使用说明：
1、在MASM目录下打开命令行窗口。
2、执行bin\ml -c -coff xxx.asm。其中asm文件是汇编代码文件。执行完毕后提到与asm文件同名的obj文件。
3、执行bin\link32 /subsystem:console xxx.obj。其中xxx.obj是上一条命令产生的obj文件。执行完毕后得到一个与源文件同名的可执行文件xxx.exe。
4、运行xxx.exe。
第2、3项的两条命令已封装在Mini-C编译器中，无需手动执行。

Note：
在部分电脑上可以会因权限不够而拒绝访问Mini-C.exe。
Mini-C.exe和productions.dat必须同时放在MASM目录下才能工作。
