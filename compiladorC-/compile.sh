bison -d yacc.y
gcc -c yacc.tab.c
flex lex.l
gcc -c lex.yy.c
gcc -c analyze.c
gcc -c symtab.c
gcc -c util.c
gcc -c main.c
gcc -c cgen.c
gcc -c code.c
gcc -c assembly.c
gcc -c binary.c
gcc yacc.tab.o lex.yy.o main.o util.o symtab.o analyze.o cgen.o code.o assembly.o binary.o -o comp.exe