/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
 void printToken( TokenType token, const char* tokenString )
 { switch (token)
   { case IF:
     case ELSE:
     case INT:
     case WHILE:
     case VOID: 
     case RETURN:
      fprintf(listing,"reserved word: %s\n",tokenString);
      break;
     case PLUS: fprintf(listing,"+\n"); break;
     case MINUS: fprintf(listing,"-\n"); break;
     case MULT: fprintf(listing,"*\n"); break;
     case DIV: fprintf(listing,"/\n"); break;
     case LT: fprintf(listing,"<\n"); break;
     case GT: fprintf(listing,">\n"); break;
     case LET: fprintf(listing,"<=\n"); break;
     case GET: fprintf(listing,">=\n"); break;
     case EQUAL: fprintf(listing,"==\n"); break;
     case DIFF: fprintf(listing,"!=\n"); break;
     case OpenP: fprintf(listing,"(\n"); break;
     case CloseP: fprintf(listing,")\n"); break;
     case OpenB: fprintf(listing,"[\n"); break;
     case CloseB: fprintf(listing,"]\n"); break;
     case OpenCB: fprintf(listing,"{\n"); break;
     case CloseCB: fprintf(listing,"}\n"); break;
     case ASSIGN: fprintf(listing,"=\n"); break;
     case SC: fprintf(listing,";\n"); break;
     case COMMA: fprintf(listing,",\n"); break;
     case EOF: fprintf(listing,"EOF\n"); break;
     case NUM:
       fprintf(listing,
           "NUM, val= %s\n",tokenString);
       break;
     case ID:
       fprintf(listing,
           "ID, name= %s\n",tokenString);
       break;
     case ERROR:
       fprintf(listing,
           "ERROR: %s\n",tokenString);
       break;
     default: /* should never happen */
       fprintf(listing,"Unknown token: %d\n",token);
   }
 }

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 */
TreeNode * newStmtNode(StmtKind kind)
{ TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = StmtK;
    t->kind.stmt = kind;
    t->lineno = lineno;
    //t->scope = "global";
  }
  //fprintf(listing,"Exp  %s %s\n",t->attr.name,t->scope);
  return t;
}

/* Function newExpNode creates a new expression
 * node for syntax tree construction
 */
TreeNode * newExpNode(ExpKind kind)
{ 
  TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else {
    for (i=0;i<MAXCHILDREN;i++)
      t->child[i] = NULL;
    t->sibling = NULL;
    t->nodekind = ExpK;
    t->kind.exp = kind;
    t->lineno = lineno;
    t->type = Void;
    //t->scope = "gcd";
  }
  return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 */
char * copyString(char * s)
{ int n;
  char * t;
  if (s==NULL) return NULL;
  n = strlen(s)+1;
  t = malloc(n);
  if (t==NULL)
    fprintf(listing,"Out of memory error at line %d\n",lineno);
  else strcpy(t,s);
  return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 */
static int indentno = 0;
static int firstLine = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=1   //Monta graficamente a identacao da arvore sintatica
#define UNINDENT indentno-=1

/* printSpaces indents by printing spaces */
static void printSpaces(void)
{ int i;
  for (i=0;i<indentno;i++)
    //fprintf(listing, "%d",indentno);
    fprintf(listing,"\t");
}

/* procedure printTree prints a syntax tree to the
 * listing file using indentation to indicate subtrees
 */
 void printTree( TreeNode * tree )
 { int i;
    if(firstLine == 0){
      firstLine = 1;
    }
    else if (firstLine==1){
      INDENT;
    }
   while (tree != NULL) {
     printSpaces();
     if (tree->nodekind==StmtK)
     { switch (tree->kind.stmt) {
         case IfK:
           fprintf(listing,"If\n");
           break;
         case AssignK:
           fprintf(listing,"Assign: \n");
           break;
         case WhileK:
           fprintf(listing,"While\n");
           break;
         case ReturnK:
           fprintf(listing,"Return\n");
           break;
         default:
           fprintf(listing,"Unknown ExpNode kind\n");
           break;
       }
     }
     else if (tree->nodekind==ExpK)
     { switch (tree->kind.exp) {
         case OpK:
           fprintf(listing,"Op: ");
           printToken(tree->attr.op,"\0");
           break;
         case ConstK:
           fprintf(listing,"Const: %d\n",tree->attr.val);
           break;
         case IdK:
           fprintf(listing,"Id: %s\n",tree->attr.name);
           break;
         case CallK:
           fprintf(listing,"Call func: %s\n",tree->attr.name);
           break;
         case TypeK:
           fprintf(listing,"Type: %s\n",tree->attr.name);
           break;
         case FuncK:
           fprintf(listing,"Func: %s\n",tree->attr.name);
           break;
         case ParamK:
           fprintf(listing,"Param: %s\n", tree->attr.name);
           break;
         case  VarK:
           fprintf(listing,"Var: %s\n",tree->attr.name);
           break;
         default:
           break;
       }
     }
     else fprintf(listing,"Unknown node kind\n");
     for (i=0;i<MAXCHILDREN;i++){
          printTree(tree->child[i]);
     }
     tree = tree->sibling;
   }
   UNINDENT;
 }
