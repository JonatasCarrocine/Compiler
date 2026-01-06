/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
#include <stdio.h>
#include "globals.h"
#include "symtab.h"
#include "analyze.h"

char* escopo = "global"; 

/* counter for variable memory locations */
static int location = 0;

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"ERRO SEMANTICO: '%s' LINHA: %d\n",message,t->lineno);
  Error = TRUE;
}

/* Procedure traverse is a generic recursive
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t, void (* preProc) (TreeNode *), void (* postProc) (TreeNode *) ){
  if (t != NULL){ 
    if (t->child[0] != NULL){
      if(t->child[0]->kind.exp == FuncK) {
        escopo = t->child[0]->attr.name;
      } 
    }
    preProc(t);
    {
      int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    if(t->child[0]!= NULL && t->child[0]->kind.exp == FuncK) escopo = "global";
    postProc(t);
    traverse(t->sibling,preProc,postProc);
    
  }
}


/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts
 * identifiers stored in t into
 * the symbol table
 */
static void insertNode( TreeNode * t)
{
  //fprintf(listing,"%s %s\n",t->attr.name,t->escopo);
  switch (t->nodekind){
    case StmtK:
      if(t->kind.stmt == AssignK){
          if (st_lookup(t->child[0]->attr.name) == -1){
            fprintf(listing,"ERRO SEMANTICO: '%s' LINHA: %d\n", t->child[0]->attr.name, t->lineno);
            Error = TRUE;
          }
          else{
            st_insert(t->child[0]->attr.name,t->lineno,0,escopo,intDataType,var);
          }
          t->child[0]->add = 1; 
      }
      break;
    case ExpK:
      switch(t->kind.exp)
      {
        case IdK:
          if(t->add != 1){
            if (st_lookup(t->attr.name) == -1){
              fprintf(listing,"ERRO SEMANTICO: '%s' LINHA: %d\n", t->attr.name, t->lineno);
              Error = TRUE;
            }
            else {
              st_insert(t->attr.name,t->lineno,0, escopo,intDataType,fun);
            }
          }
          break;
        case TypeK:
          if(t->child[0] != NULL){
            switch (t->child[0]->kind.exp)
            {
              case  VarK:
                if (st_lookup(t->attr.name) == -1){ /* não encontrado na tabela, inserir*/
                  st_insert(t->child[0]->attr.name,t->lineno,location++, escopo,intDataType, var);
                  //fprintf(listing,"Tipo: %s %s\n", t->child[0]->attr.name, escopo);
                  }
                else{ /* encontrado na tabela, verificar escopo */
                  st_insert(t->child[0]->attr.name,t->lineno,0, escopo,intDataType, var);
                  //fprintf(listing,"Variavel: %s\n", t->child[0]->attr.name);
                }
                break;
              case FuncK: /* encontrado na tabela, verificar escopo */
                if (st_lookup(t->attr.name) == -1){
                  st_insert(t->child[0]->attr.name,t->lineno,location++, "global",t->child[0]->type,fun);
                   //fprintf(listing,"Variavel: %s %s %d\n", t->attr.name, escopo, t->lineno);
                   }
                else
                  fprintf(listing,"ERRO SEMANTICO: '%s'. LINHA: %d\n", t->child[0]->attr.name, t->lineno); //mais de uma declaracao de funcao
                break;
              default:
                break;

            }
          }
          break;
        case CallK:
          if (st_lookup(t->attr.name) == -1 && strcmp(t->attr.name, "input") != 0 && strcmp(t->attr.name, "output") != 0){
            fprintf(listing,"ERRO SEMANTICO: '%s'.' LINHA: %d\n", t->attr.name, t->lineno); // Funcao nao declarada
            Error = TRUE;
          }
          else 
            st_insert(t->attr.name,t->lineno,0, escopo,0,fun);
          break;
        case ParamK:
          st_insert(t->attr.name,t->lineno,location++, escopo,intDataType, var);
          //fprintf(listing,"Parametro: %s %s %d\n", t->attr.name, escopo, t->lineno);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 
  st_insert("input", 0, 0, "global", intDataType, fun);
  st_insert("output", 0, 0, "global", voidDataType, fun);
  traverse(syntaxTree,insertNode,nullProc);
  typeCheck(syntaxTree);
  findMain();
  if (TraceAnalyze && !Error ){ 
    fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}


/* Procedure checkNode performs
 * type checking at a single tree node
 */
void checkNode(TreeNode * t)
{
  switch (t->nodekind)
  { 
    case ExpK:
      switch (t->kind.exp)
      { case OpK:
        
          if (((t->child[0]->kind.exp == CallK) &&( getFunType(t->child[0]->attr.name)) == voidDataType) ||
              ((t->child[1]->kind.exp == CallK) && (getFunType(t->child[1]->attr.name) == voidDataType)))
                typeError(t->child[0],"Acao invalida devido funcao VOID ");
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      {
        case AssignK:
          if (t->child[1]->kind.exp == CallK && getFunType(t->child[1]->attr.name) == voidDataType)
            typeError(t->child[0],"Atribuicao com funcao VOID");
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Procedure typeCheck performs type checking
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree){ 
  traverse(syntaxTree,checkNode, nullProc);
}

