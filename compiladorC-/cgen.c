#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"
#include "parse.h"
#include "analyze.h"

/* tmpOffset is the memory offset for temps
   It is decremented each time a temp is
   stored, and incremeted when loaded again
*/
static int tmpOffset = 0;

/* prototype for internal recursive code generator */
static void cGen(TreeNode *tree);

QuadList head = NULL;

int location = 0;
int mainLocation;
int nparams = -1;
int nlabel = 0;
int ntemp = 0;

/*Atribuicao de umas structs do tipo Address*/
Address empty, aux, var1, offset;

/*Vetor de atribuicoes para o codigo intermediario*/
const char *OpKindNames[] = {"ADD", "SUB", "MUL", "DIV", "LT", "EQUAL", "LEQUAL", "GT", "GREQUAL", "AND", "OR", "ASSIGN", "ALLOC", "IMMED", "LOAD", "STORE",
                             "VEC", "GOTO", "IFF", "RET", "FUN", "END", "PARAM", "CALL", "ARG", "LAB", "HLT"};

/*Funcao pra inserir as quadruplas*/
void quadIns(OpKind op, Address addr1, Address addr2, Address opRes)
{
  Quad quad;
  quad.op = op;
  quad.addr1 = addr1;
  quad.addr2 = addr2;
  quad.opRes = opRes;
  QuadList novo = (QuadList)malloc(sizeof(struct QuadListRec));
  novo->location = location;
  novo->quad = quad;
  novo->next = NULL;
  if (head == NULL){ /*Se nao tiver uma quadrupla nova, aponta pra ela mesma*/
    head = novo;
  }
  else{ /*Aponta pra proxima quadrupla*/
    QuadList q = head;
    while (q->next != NULL)
      q = q->next;
    q->next = novo;
  }
  location++;
}

/*Funcao pra atualizar as quadruplas*/
int quadUp(int loc, Address addr1, Address addr2, Address opRes)
{
  QuadList q = head;
  while (q != NULL){
    if (q->location == loc)
      break;
    q = q->next;
  }
  if (q == NULL)
    return 0;
  else{
    q->quad.addr1 = addr1;
    q->quad.addr2 = addr2;
    q->quad.opRes = opRes;
    return 1;
  }
}

 // Função pra criar novo registrador temporário
char * novoTemp()
{
  char *temp = (char *) malloc((ntemp_size + 3) * sizeof(char)); //Alocar espaco pro registrador temporario
  sprintf(temp, "$t%d", ntemp);
  ntemp = (ntemp + 1) % nregtemp;
  return temp;
}

// Função para criar novo label (rotulo)
char * novoLabel()
{
  char *label = (char *) malloc((nlabel_size + 3) * sizeof(char)); //Alocar espaco pro label
  sprintf(label, "L%d", nlabel);
  nlabel++;
  return label;
}

//Funções da pilha de simbolos 
//Criando um endereco vazio
Address addrCriaVazio()
{
  Address addr;
  addr.kind = Empty;
  addr.contents.var.name = NULL;
  addr.contents.var.scope = NULL;
  return addr;
}

//Criacao de um endereco constante
Address addrCreateIntConst(int val)
{
  Address addr;
  addr.kind = IntConst;
  addr.contents.val = val;
  return addr;
}

//Criacao de uma string
 Address addrCreateString(char *name, char *scope)
{
  Address addr;
  addr.kind = String;
  addr.contents.var.name = (char *) malloc(strlen(name) * sizeof(char)); //Aloca espaco pro nome
  strcpy(addr.contents.var.name,name);
  if(scope == NULL){
      //printf("Entrou na condição é NULL \n");
    addr.contents.var.scope = (char *) malloc(strlen(name) * sizeof(char)); //Aloca espaco pro escopo
    strcpy(addr.contents.var.scope,name);}
  else {
    addr.contents.var.scope = (char *) malloc(strlen(scope)* sizeof(char));
    strcpy(addr.contents.var.scope,scope);
  }
  return addr;
}

/* Procedure genStmt generates code at a statement node */
static void genStmt(TreeNode *tree)
{
  TreeNode *p1, *p2, *p3;
  Address addr1, addr2, opRes; //Principais
  Address aux1, aux2; //Auxiliares
  int loc1, loc2, loc3;
  char *label;
  char *temp;

  switch (tree->kind.stmt){ //Tipo statement
  case IfK: //Se for if
    if (TraceCode)
      emitComment("-> if");
    p1 = tree->child[0]; 
    p2 = tree->child[1]; //if true
    p3 = tree->child[2]; // if false (entra no else)
    // condicao if
    cGen(p1);
    addr1 = aux;
    // if false
    loc1 = location;
    quadIns(opIFF, addr1, empty, empty);
    // if true
    cGen(p2);
    
    loc2 = location;
    quadIns(opGOTO, empty, empty, empty); //jump else
    // end if
    label = novoLabel(); //Cria uma nova label
    quadIns(opLAB,addrCreateString(label, tree->scope), empty, empty);
    // if false comes to here
    quadUp(loc1, addr1,addrCreateString(label, tree->scope), empty);
    // else
    cGen(p3);
    if (p3 != NULL){
      // vai para o final
      loc3 = location;
      quadIns(opGOTO, empty, empty, empty); //sair else
    }
    label = novoLabel(); //Cria uma nova label
    // fchegou no final
    quadIns(opLAB,addrCreateString(label, tree->scope), empty, empty);
    quadUp(loc2,addrCreateString(label, tree->scope), empty, empty);
    if (p3 != NULL)
      quadUp(loc3,addrCreateString(label, tree->scope), empty, empty);
    if (TraceCode)
      emitComment("<- if");
    break;

  case WhileK: //Caso seja while
    if (TraceCode)
      emitComment("-> while");
    p1 = tree->child[0];
    p2 = tree->child[1];
    // inicio do while
    label = novoLabel();
    quadIns(opLAB,addrCreateString(label, tree->scope), empty, empty); //só conhecemos o rótulo(label) no final do stmt
    // condicao while
    cGen(p1); //chama recursiva
    addr1 = aux;
    // if condition is false
    loc1 = location;
    quadIns(opIFF, addr1, empty, empty);
    // while
    cGen(p2); //body
    loc3 = location;
    quadIns(opGOTO,addrCreateString(label, tree->scope), empty, empty); //return to statement till the condition is false
    // final
    label = novoLabel();
    quadIns(opLAB,addrCreateString(label, tree->scope), empty, empty); //here you know the label
  //se a condição if foir falsa, executa quadUp ( para atualizar)
    quadUp(loc1, addr1,addrCreateString(label, tree->scope), empty);//update quad bc you r in the end
    if (TraceCode)
      emitComment("<- while");
    break;

  case AssignK: //Atribuicao
    if (TraceCode)
      emitComment("-> atrib");
    p1 = tree->child[0];//arg
    p2 = tree->child[1];//body
    // var
    cGen(p1);
    addr1 = aux;
    aux1 = var1;
    aux2 = offset;
    // exp
    cGen(p2);
    addr2 = aux;
    quadIns(opASSIGN, addr1, addr2, empty);
    quadIns(opSTORE, aux1, addr1, aux2);
    if (TraceCode)
      emitComment("<- atrib");
    break;

  case ReturnK: //Comando Return
    if (TraceCode)
      emitComment("-> return");
    p1 = tree->child[0];
    cGen(p1);
    // // se tiver retorno entra no if ( retorna um inteiro)
    if (p1 != NULL)
      addr1 = aux;
    //se não tem retorno
    else
      addr1 = empty;
    quadIns(opRET, addr1, empty, empty);
    if (TraceCode)
      emitComment("<- return");
    break;

  default:
    break;
  }
} /* genStmt */

/* Funcao genExp gera o codigo em um no de expressao*/
static void genExp(TreeNode *tree)
{
  TreeNode *p1, *p2, *p3;
  Address addr1, addr2, opRes;
  int loc1, loc2, loc3;
  char *label;
  char *temp;
  char *s = "";
  
  switch (tree->kind.exp){
    case ConstK:
      //printf("\nEntrou ConstK\n");
      if (TraceCode)
        emitComment("-> Const");
      addr1 = addrCreateIntConst(tree->attr.val);
      temp = novoTemp();
      aux =addrCreateString(temp, tree->scope);
      quadIns(opIMMED, aux, addr1, empty);// imediato
      if (TraceCode)
        emitComment("<- Const");
      break;

    case IdK:
      //printf("\nEntrou IdK\n");
      if (TraceCode)
        emitComment("-> Id");
      aux = addrCreateString(tree->attr.name, tree->scope);
      // printf("tree size: %d \n", tree->size);
      p1 = tree->child[0];
      if (p1 != NULL)
      {
        //printf("entrou no vec- é um vetor \n");
        temp = novoTemp();
        addr1 =addrCreateString(temp, tree->scope);
        addr2 = aux;
        cGen(p1);
        quadIns(opVEC, addr1, addr2, aux);
        var1 = addr2;
        offset = aux;
        aux = addr1;
      }
      else
      {
        // printf("não entrou no vec - não é um vetor\n");
        temp = novoTemp();
        addr1 =addrCreateString(temp, tree->scope);
        quadIns(opLOAD, addr1, aux, empty);
        var1 = aux;
        offset = empty;
        aux = addr1;
      }
      if (TraceCode)
        emitComment("<- Id");
      break;

    case TypeK:
      //printf ("\n entrou no TypeK");
      p1 = tree->child[0];
      //printf("%d\n",p1);
      cGen(p1);
      break;

    case FuncK:
      //printf ("\n entrou no FunDeclK");
      //printf("\nFundeclK: %s \n",tree->attr.name);
      if (TraceCode)
        emitComment("-> Fun");
      // if main
      if (strcmp(tree->attr.name, "main") == 0)
        mainLocation = location;
      if ((strcmp(tree->attr.name, "input") != 0) && (strcmp(tree->attr.name, "output") != 0))
      {
        quadIns(opFUN, addrCreateString(tree->attr.name, tree->scope), empty, empty);
        // params
        //printf("\nEntrou aqui!!");
        p1 = tree->child[0];
        cGen(p1);
        // decleração  & expressoes
        p2 = tree->child[1];
        cGen(p2);
    
        quadIns(opEND, addrCreateString(tree->attr.name, tree->scope), empty, empty);
      }
      if (TraceCode)
        emitComment("<- Fun");
      break;

    case CallK: //Se for uma CALL
      //printf("Entrou na AtivK \n");
      if (TraceCode)
        emitComment("-> Call");
      //Address a1 = addrCreateIntConst(tree->params);
      // é um parametro
      nparams = tree->params;
      p1 = tree->child[0];
      while (p1 != NULL)
      {
        cGen(p1);
        quadIns(opPARAM, aux, empty, empty);
        nparams--;
        p1 = p1->sibling;
      }
      nparams = -1;
      temp = novoTemp();
      aux =addrCreateString(temp, tree->scope);
      quadIns(opCALL, addrCreateString(tree->attr.name, tree->scope), addrCreateIntConst(tree->params), aux);

      if (TraceCode)
        emitComment("<- Call");
      break;

    case ParamK: //Se for um parametro
      //printf("(%s), (%s)\n",tree->attr.name, tree->scope);
      if (TraceCode)
        emitComment("-> Param");
      quadIns(opARG, addrCreateString(tree->attr.name, tree->scope), empty, addrCreateString(tree->scope,tree->scope));
      if (TraceCode)
        emitComment("<- Param");
      break;

    case VarK: //Se for uma variavel
      //printf(" ENtrou na VardeclK \n");
      if (TraceCode)
        emitComment("-> Var");
      if (tree->size != 0){
        quadIns(opALLOC, addrCreateString(tree->attr.name, tree->scope), addrCreateString(tree->scope,tree->scope), addrCreateIntConst(tree->size));
      }
      else
        quadIns(opALLOC, addrCreateString(tree->attr.name, tree->scope), addrCreateString(tree->scope,tree->scope), addrCreateIntConst(1));
      if (TraceCode)
        emitComment("<- Var");
      break;

    case OpK: //Se for uma operacao
      //printf("\nEntrou OpK");
      // printf("OpK: %d \n",tree->attr.op);
      if (TraceCode)
        emitComment("-> Op");
      p1 = tree->child[0];
      p2 = tree->child[1];

      cGen(p1);
      addr1 = aux;

      cGen(p2);
      addr2 = aux;
      temp = novoTemp();
      aux =addrCreateString(temp, tree->scope);

    switch (tree->attr.op){  // Casos baseado no util.c desenvolvido
      case PLUS: //Soma
        // printf("soma \n");
        quadIns(opADD, addr1, addr2, aux);
        break;
      case MINUS: //Subtracao
        //printf("sub\n");
        quadIns(opSUB, addr1, addr2, aux);
        break;
      case MULT: //Multiplicacao
        //printf("mult\n");
        quadIns(opMULT, addr1, addr2, aux);
        break;
      case DIV: //Divisao
        //printf("over\n");
        quadIns(opDIV, addr1, addr2, aux);// op /
        break;
      case LT: //Menor que
        //printf("lt\n");
        quadIns(opLT, addr1, addr2, aux);
        break;
      case LET: //Menor ou igual a
        //printf("leq\n");
        quadIns(opLEQUAL, addr1, addr2, aux);
        break;
      case GT: //Maior que
        //printf("gt\n");
        quadIns(opGT, addr1, addr2, aux);
        break;
      case GET: //Maior ou igual a
        //printf("greq\n");
        quadIns(opGREQUAL, addr1, addr2, aux);
        break;
      case EQUAL: //Igual a
        //printf("equal\n");
        quadIns(opEQUAL, addr1, addr2, aux);
        break;
      case DIFF: //Diferente de
        //printf("dif\n");
        quadIns(opGT, addr1, addr2, aux);
        opRes = aux;
        temp = novoTemp();
        aux =addrCreateString(temp, tree->scope);
        quadIns(opLT, addr1, addr2, aux);
        addr1 = opRes;
        addr2 = aux;
        temp = novoTemp();
        aux =addrCreateString(temp, tree->scope);
        //quadIns(opOR, aux, addr1, addr2);
        break;
      default:
        emitComment("BUG: Unknown operator");
        break;
    }
    if (TraceCode)
      emitComment("<- Op");
    break;

  default:
    break;
  }
} /* genExp */

/* Procedure cGen recursively generates code by
 * tree traversal
 */
static void cGen(TreeNode *tree)
{
  if (tree != NULL){
      //printf("(%s) (%s)\n" ,tree->attr.name, tree->scope);
        /*if(tree->kind.exp == VarK ){
            printf(" \n%s" ,tree->attr.name);
        }*/
    switch (tree->nodekind){
      case StmtK: /*Entrou no tipo Statement*/
        genStmt(tree);
        break;
      case ExpK: /*Entrou no tipo Expression*/
        genExp(tree);
        break;
      default:
        break;
    }
    if (nparams == -1){
      cGen(tree->sibling); //No irmao
    }
    else{
      if (nparams == 0){
        cGen(tree->sibling); //No irmao
      }
    }
  } 
}

/*Funcao pra imprimir as quadruplas*/
void Quad_Print()
{
  //printf("\nentrou na funcao imprime codigo intermediario \n");
  QuadList q = head;
  Address a1, a2, a3;
  while (q != NULL){
    a1 = q->quad.addr1;
    a2 = q->quad.addr2;
    a3 = q->quad.opRes;
    printf("(%s, ", OpKindNames[q->quad.op]);
    switch (a1.kind){ //Segundo componente da quadrupla
      case Empty: //Se for um campo vazio 
        printf("_");
        break;
      case IntConst: //Se for um valor inteiro constante
        printf("%d", a1.contents.val);
        break;
      case String: //Se for uma string
        printf("%s", a1.contents.var.name);
        break;
      default:
        break;
    }
    printf(", ");
    switch (a2.kind){ //Terceiro componente da quadrupla
      case Empty:
        printf("_");
        break;
      case IntConst:
        printf("%d", a2.contents.val);
        break;
      case String:
        printf("%s", a2.contents.var.name);
        break;
      default:
        break;
    }
    printf(", ");
    switch (a3.kind){ //Ultimo componente da quadrupla
      case Empty:
        printf("_");
        break;
      case IntConst:
        printf("%d", a3.contents.val);
        break;
      case String:
        printf("%s", a3.contents.var.name);
        break;
      default:
        break;
    }
    printf(")\n");
    q = q->next; //Aponta pra próxima quadrupla
  }
  //printf("\nImprimiu!!!!!\n");
}

/*Funcao principal pra chamar demais funcoes e gerar o codigo intermediario*/
void codeGen(TreeNode *syntaxTree, char *codefile)
{
  char *s = malloc(strlen(codefile) + 7);
  strcpy(s, "Arquivo: ");
  strcat(s, codefile);
  emitComment("\nC- Codigo Intermediario - Cmenos");
  emitComment(s);

  empty = addrCriaVazio();
  cGen(syntaxTree);
  quadIns(opHLT, empty, empty, empty);
  printf("\n");
  Quad_Print();
  emitComment("End of execution"); //Finalizacao
}

QuadList getIntermediate()
{
  return head;
}
