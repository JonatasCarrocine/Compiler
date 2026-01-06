%option noyywrap
%option nounput
%{
#include "globals.h"
#include "util.h"
#include "scan.h"

/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
extern int yylex(void);
%}
%option noyywrap

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t\r]+

%%

"if" {return IF;}
"else" {return ELSE;}
"int" {return INT;}
"while" {return WHILE;}
"void" {return VOID;}
"return" {return RETURN;}
"+" {return PLUS;}
"-" {return MINUS;}
"*" {return MULT;}
"/" {return DIV;}
"<" {return LT;}
">" {return GT;}
"<=" {return LET;}
">=" {return GET;}
"==" {return EQUAL;}
"!=" {return DIFF;}
"(" {return OpenP;}
")" {return CloseP;}
"[" {return OpenB;}
"]" {return CloseB;}
"{" {return OpenCB;}
"}" {return CloseCB;}
"=" {return ASSIGN;}
";" {return SC;}
"," {return COMMA;}
{whitespace}  {}
<<EOF>> {return EOF;}
{newline} {lineno++;}
{identifier} {return ID;}
{number}  {return NUM;}
"/*"             { char c;
                  char p='\0';
                  do
                  { c = input();
                    if (c == EOF) break;
                    if (c == '\n') lineno++;
                    if (p == '*' && c == '/') break;
                    p = c;
                  } while (1);
                }
.               {return ERROR;}


%%
int getToken(void)
{ static int firstTime = TRUE;
  int Token;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  Token = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(Token,tokenString);
  }
  

  return Token;
}
