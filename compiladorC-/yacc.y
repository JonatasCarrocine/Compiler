/****************************************************/
/* File: cmenos.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void);
int yyerror(char *msg);
char * scope = "Global";
char * idtype = "";
char * datatype = "";
static char *savedname;
int flag = 0;
int params = 0;


%}

%token IF ELSE WHILE RETURN VOID
%right INT
%token ASSIGN EQUAL DIFF LT GT GET LET PLUS MINUS MULT DIV OpenP CloseP OpenB CloseB OpenCB CloseCB COMMA SC
%token ID NUM
%token ERROR ENDFILE

/*daqui pra baixo tem novo coisas*/
%nonassoc CloseP
%nonassoc ELSE

%% /* Grammar for cmenos */

program     : dec-list
                 { savedTree = $1;}
            ;

dec-list : dec-list dec
          {
            YYSTYPE t = $1;
              if (t != NULL){
                while (t->sibling != NULL)
                   t = t->sibling;
                t->sibling = $2;
                $$ = $1;
              }
              else $$ = $2;
          }
	      | dec { $$ = $1; }
	      ;

dec 	    : var-dec { $$ = $1 ;}
	    | fun-dec { $$ = $1;
      scope = "Global"; }
	    ;

identificador : ID
                {
                  $$ = newExpNode(IdK);
                  $$->attr.name = copyString(tokenString);
                  savedname = copyString(tokenString);
                  $$->scope = scope;
                  if(flag ==  1){
                    scope= savedname;
                    flag = 0;
                  }
                }
              ;
numero : NUM
          {
            $$ = newExpNode(ConstK);
            $$->type = intDataType;
            $$->attr.val = atoi(tokenString);
          }
var-dec : INT identificador
          SC
          {
            $$ = newExpNode(TypeK);
            $$->attr.name = "INT";
            $$->size = 0;
            $$->child[0] = $2;
            $$->scope= scope;
            $2->kind.exp = VarK;
            $2->type = intDataType;
          }
	      | INT identificador OpenB numero CloseB SC
            {
              $$ = newExpNode(TypeK);
              $$->attr.name = "INT";
              $$->child[0] = $2;
              $$->scope = scope;
              $2->kind.exp = VarK;
              $2->size = $4->attr.val;
              $2->type = intDataType;
            } | error {yyerrok;}
	      ;

fun-dec : INT identificador OpenP params CloseP composto-dec
            {
              $$ = newExpNode(TypeK);
              $$->attr.name = "INT";
              $$->child[0] = $2;
              $$->scope = scope;
              $2->kind.exp = FuncK;
              $2->lineno = $$->lineno;
              $2->type = intDataType;
              $2->child[0] = $4;
              $2->child[1] = $6;
              flag = 1;
            }
        | VOID identificador OpenP params CloseP composto-dec
                    {
                      $$ = newExpNode(TypeK);
                      $$->attr.name = "VOID";
                      $$->child[0] = $2;
                      $$->scope = scope;
                      $2->type = voidDataType;
                      $2->kind.exp = FuncK;
                      $2->lineno = $$->lineno;
                      $2->child[0] = $4;
                      $2->child[1] = $6;
                      flag =1 ;
                    }
        ;

params : param-list { $$ = $1; }
       | VOID
          {
            $$ = newExpNode(TypeK);
            $$->attr.name = "VOID";
            $$->size = 0;
            $$->child[0] = NULL;
          }
       ;

param-list : param-list COMMA param-list
              {
                YYSTYPE t = $1;
                if (t != NULL){
                  while (t->sibling != NULL)
                       t = t->sibling;
                  t->sibling = $3;
                  $$ = $1;
                }
                else $$ = $3;
              }
           | param { $$ = $1; }
           ;

param : tipo-espec identificador
        {
          $$ = $1;
          $$->child[0] = $2;
          $$->scope = scope;
          $2->kind.exp = ParamK;
          $$->size = 0;
          flag =0;
        }
      | tipo-espec identificador OpenB CloseB
        {
          $$ = $1;
          $$->child[0] = $2;
          $$->scope = scope;
          $2->kind.exp = ParamK; 
          $$->size = 0;
          flag=0;
        }
      ;

tipo-espec  : INT
              {
                $$ = newExpNode(TypeK);
                $$->attr.name = "INT";
                $$->type = intDataType;
                $$->size = 1;
              }
            | VOID
              {
                $$ = newExpNode(TypeK);
                $$->attr.name = "VOID";
                $$->type = intDataType;
                $$->size = 1;
              }
            ;


composto-dec : OpenCB local-dec stmt-list CloseCB
              {
                YYSTYPE t = $2;
                  if (t != NULL){
                    while (t->sibling != NULL)
                       t = t->sibling;
                    t->sibling = $3;
                    $$ = $2;
                  }
                  else $$ = $3;
              }
             | OpenCB CloseCB {}
             | OpenCB  local-dec CloseCB { $$ = $2; }
             | OpenCB stmt-list CloseCB { $$ = $2; }
             ;

local-dec : local-dec var-dec
            {
              YYSTYPE t = $1;
                if (t != NULL){
                  while (t->sibling != NULL)
                     t = t->sibling;
                  t->sibling = $2;
                  $$ = $1;
                }
                else $$ = $2;
            }
          | var-dec { $$ = $1; }
          ;

stmt-list : stmt-list stmt
            {
              YYSTYPE t = $1;
              if (t != NULL){
                while (t->sibling != NULL)
                t = t->sibling;
                t->sibling = $2;
                $$ = $1;
              }
              else $$ = $2;
            }
          | stmt { $$ = $1; }
          ;

stmt : exp-dec { $$ = $1; }
     | composto-dec { $$ = $1; }
     | sel-dec { $$ = $1; }
     | it-dec { $$ = $1; }
     | retorno-dec { $$ = $1; }
     ;

exp-dec : exp SC { $$ = $1; }
        |  SC {} | error {yyerrok;}
        ;

sel-dec : IF OpenP exp CloseP stmt
          {
            $$ = newStmtNode(IfK);
            $$->child[0] = $3;
            $$->child[1] = $5;
            $$->scope = $3->scope;
          }
        | IF OpenP exp CloseP stmt ELSE stmt
          {
            $$ = newStmtNode(IfK);
            $$->child[0] = $3;
            $$->child[1] = $5;
            $$->child[2] = $7;
            $$->scope= $3->scope;
          }
        ;

it-dec : WHILE OpenP exp CloseP stmt
        {
          $$ = newStmtNode(WhileK);
          $$->child[0] = $3;
          $$->child[1] = $5;
          $$->scope = $3->scope;
        }
        ;

retorno-dec : RETURN SC { $$ = newStmtNode(ReturnK); }
            | RETURN exp SC
              {
                $$ = newStmtNode(ReturnK);
                $$->child[0] = $2;
              }
            ;

exp : var ASSIGN exp
      {
        $$ = newStmtNode(AssignK);
        $$->attr.name= $1->attr.name;
        $$->scope = scope;
        $$->child[0] = $1;
        $$->child[1] = $3;
      }
    | simples-exp { $$ = $1; }
    ;

var : identificador { $$ = $1; }
    | identificador OpenB exp CloseB
      {
        $$=$1;
       // $$ = novoExpNode(VetorK);
        $$->attr.name = $1->attr.name;
        $$->child[0] = $3;
      }
    ;

simples-exp : soma-exp relacional soma-exp
              {
                  $$ = $2;
                  $$->child[0] = $1;
                  $$->child[1] = $3;
                  $$->scope = scope;
              }
            | soma-exp { $$ = $1; }
            ;

relacional : EQUAL
              {
                $$ = newExpNode(OpK);
                $$->attr.op = EQUAL;
              }
           | LT
              {
                $$ = newExpNode(OpK);
                $$->attr.op = LT;
              }
           | GT
              {
                $$ = newExpNode(OpK);
                $$->attr.op = GT;
              }
           | GET
              {
                $$ = newExpNode(OpK);
                $$->attr.op = GET;
              }
           | LET
              {
                $$ = newExpNode(OpK);
                $$->attr.op = LET;
              }
           | DIFF
              {
                $$ = newExpNode(OpK);
                $$->attr.op = DIFF;
              }
           ;

soma-exp : soma-exp soma termo {
            $$ = $2;
            $$->child[0] = $1;
            $$->child[1] = $3;
            $$->scope = scope;
         }
         | termo { $$ = $1; }
         ;

soma : PLUS
       {
         $$ = newExpNode(OpK);
         $$->attr.op = PLUS;
       }
     | MINUS
        {
          $$ = newExpNode(OpK);
          $$->attr.op = MINUS;
        }
     ;

termo : termo mult fator
            {
              $$ = $2;
              $$->scope = scope;
              $$->child[0] = $1;
              $$->child[1] = $3;
            }
      | fator { $$ = $1; }
      ;

mult : MULT
       {
         $$ = newExpNode(OpK);
         $$->attr.op = MULT;
       }
     | DIV
        {
          $$ = newExpNode(OpK);
          $$->attr.op = DIV;
        }
     ;

fator : OpenP exp CloseP { $$ = $2; }
      | var { $$ = $1; }
      | ativ { $$ = $1;
        params = 0; }
      | numero { $$ = $1; }
      ;

ativ : identificador OpenP arg-list CloseP
        {
          $$ = newExpNode(CallK);
          $$->attr.name = $1->attr.name;
          $$->scope = scope;
          $$->child[0] = $3;
          $$->params = params;

        }
        | identificador OpenP CloseP
         {
           $$ = newExpNode(CallK);
           $$->attr.name = $1->attr.name;
           $$->scope = scope;
           $$->params = params;
         }
     ;


arg-list : arg-list COMMA exp
            {
              YYSTYPE t = $1;
              if (t != NULL){
                while (t->sibling != NULL)
                t = t->sibling;
                t->sibling = $3;
                params ++;
                $$ = $1;
              }
              else $$ = $3;
            }
         | exp { 
           params ++;
           $$ = $1; }
         ;


%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  exit(0);
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the cmenos scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}
