c-: lex.yy.c parser.tab.c printtree.cpp semantic.cpp yyerror.cpp scanType.h emitcode.cpp codegen.cpp
	g++ -g lex.yy.c parser.tab.c symbolTable.cpp printtree.cpp getopt.cpp semantic.cpp yyerror.cpp emitcode.cpp codegen.cpp -o c-

parser.tab.h parser.tab.c: parser.y
	bison -v -t -d parser.y

lex.yy.c: parser.l parser.tab.h
	flex parser.l

tar:
	tar -cvf purkettHw7.tar getopt.cpp getopt.h makefile parser.l parser.y printtree.cpp printtree.h scanType.h semantic.cpp semantic.h symbolTable.cpp symbolTable.h yyerror.h yyerror.cpp emitcode.cpp emitcode.h codegen.cpp