#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"
#include "assembly.h"

AssemblyCode headcode = NULL;
FunList funcHeadList = NULL;

int currentMemory = 0;
int currentParam = 0;
int nscopes = 0;
int currentArg = 0;
int line = 0;
int jmpMain = 0;
int nArg = 0;

const char * NomedeInstr[] =  {  "nop", "halt", "add", "addi", "sub", "mult", "divi", "mod", "and", "or", "not", "xor", "slt", "sgt", "sle", "sge",
                                "shl", "shr", "move", "ldi", "beq", "bne", "jmp", "in", "out", "str", "load", "jr" };

const char * NomedeReg[] = { "$zero", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9", "$t10", "$t11", "$t12", "$t13", "$t14",
                            "$t15", "$a0", "$a1", "$a2", "$a3", "$a4", "$a5", "$a6", "$a7", "$a8", "$a9", "$sp", "$gp", "$ra", "$ret", "$jmp" };

//Funcao para inserir a instrucao
void InserirFunc(char *id){
    FunList newList = (FunList) malloc(sizeof(struct FunListRec));
    newList-> id = (char *)malloc(strlen(id) * sizeof(char));
    strcpy(newList->id, id);
    newList->size = 0;
    newList->memloc= currentMemory;
    newList->next = NULL;
    if(funcHeadList == NULL){
        funcHeadList = newList;
    }
    else{
        FunList list = funcHeadList;
        while (list->next != NULL) list = list->next;
        list->next = newList;
    }
    nscopes ++;
}

//Funcao para inserir a variavel
void inserirVar (char * scope, char * id, int size, VarKind kind){
    FunList list = funcHeadList;
    if(scope == NULL){
        if(kind == 1){
            scope = list->id;
        }
        else{
            scope = list->next->id;
        }
    }
    while(list!=NULL && strcmp(list->id, scope) !=0)
        list = list->next;
    if(list==NULL){
        InserirFunc(scope);
        list = funcHeadList;
        while(list != NULL && strcmp(list->id, scope)!=0)
            list = list->next;
    }
    VarList newVar = (VarList) malloc(sizeof(struct VarListRec));
    newVar->id = (char *) malloc(strlen(id) * sizeof(char));
    strcpy(newVar->id, id);
    newVar->size = size;
    newVar->memloc = list->size;
    currentMemory = currentMemory + size;
    newVar->kind = kind;
    newVar->next = NULL;
    if(list->vars ==NULL){
        list->vars = newVar;
    }
    else{
        VarList v = list->vars;
        while (v->next != NULL)  v = v->next;
        v->next = newVar;
    }
    list->size = list->size + size;
}

//Funcao para inserir uma label
void inserirLabel (char * label){
    AssemblyCode new = (AssemblyCode) malloc (sizeof(struct AssemblyCodeRec));
    new->lineno = line;
    new->kind = lbl;
    new->line.label = (char *) malloc(strlen(label) * sizeof(char));
    strcpy(new->line.label, label);
    new->next = NULL;
    if(headcode == NULL){
        headcode = new;
    }
    else{
        AssemblyCode a = headcode;
        while(a->next != NULL) a = a->next;
        a->next = new;
    }
}

//Funcao para inserir uma instrucao
void InserirInstrucao(InstrFormat format, InstrType op, Reg reg1, Reg reg2, Reg reg3, int imm, char * imlbl){
    Instruction in;
    in.format = format;
    in.opcode = op;
    in.reg1 = reg1;
    in.reg2 = reg2;
    in.reg3 = reg3;
    in.im = imm;
    if(imlbl != NULL){
        in.imlbl = (char *) malloc(strlen(imlbl)* sizeof(char));
        strcpy(in.imlbl, imlbl);
    }
    AssemblyCode novo = (AssemblyCode) malloc(sizeof(struct AssemblyCodeRec));
    novo->lineno = line;
    novo->kind = instr;
    novo->line.instruction = in;
    novo->next = NULL;
    if (headcode == NULL) {
        headcode = novo;
    }
    else {
        AssemblyCode a = headcode;
        while (a->next != NULL) a = a->next;
        a->next = novo;
    }
    line ++;
}

//Funcao do formato de instrucao
void instructionFormat1(InstrType opcode, Reg reg1, Reg reg2, Reg reg3){
    InserirInstrucao(format1, opcode, reg1, reg2, reg3, 0, NULL);
}

void instructionFormat2(InstrType opcode, Reg reg1, Reg reg2, int imm, char * imlbl){
    InserirInstrucao(format2, opcode, reg1, reg2, $zero, imm, imlbl);
}

void instructionFormat3(InstrType opcode, Reg reg1, int imm, char * imlbl){
    InserirInstrucao(format3, opcode, reg1, $zero, $zero, imm, imlbl);
}

void instructionFormat4(InstrType opcode, int imm, char * imlbl){
    InserirInstrucao(format4, opcode, $zero, $zero, $zero, imm, imlbl);
}

Reg getParamReg (){
    return (Reg) 1 + nregtemp + currentParam;
}

Reg getReg (char * regName){
    for(int i = 0; i< nregisters; i++){
        if(strcmp(regName, NomedeReg[i])==0){
            return (Reg) i;
        }
    }
    return $zero;
}

Reg getArgReg (){
    return (Reg) 1 + nregtemp + currentArg;
}

int getLabelLine (char * label){
    AssemblyCode a = headcode;
    while(a->next != NULL){
        if(a->kind == lbl && strcmp(a->line.label, label) == 0) return getAdjustedLineno(a->lineno);
        a = a->next;
    }
    return -1;
}

VarKind checkType (QuadList l){
    QuadList aux = l;
    Quad q = aux->quad;
    aux = aux->next;
    while(aux!= NULL && aux->quad.op != opEND){
        if(aux->quad.op == opVEC && strcmp(aux->quad.addr2.contents.var.name, q.addr1.contents.var.name) == 0) return address;
        aux = aux->next;
    }
    return simple;
}

int getVarMemLoc (char *id, char *scope){
    FunList f = funcHeadList;
    while (f != NULL && strcmp(f->id, scope) != 0)f = f->next;
    if (f == NULL)
        return -1;
    VarList v = f->vars;
    while (v != NULL) {
        if (strcmp(v->id, id) == 0) return v->memloc;
        v = v->next;
    }
    return -1;
}

VarKind getVarKind (char *id, char *scope){
    FunList f = funcHeadList;
    while (f != NULL && strcmp(f->id, scope) != 0) f = f->next;
    if (f == NULL) {
        return simple;
    }
    VarList v = f->vars;
    while (v != NULL) {
        if (strcmp(v->id, id) == 0) return v->kind;
        v = v->next;
    }
    return simple;
}

//Tamanho da funcao
int getFunSize (char *id){
    FunList f = funcHeadList;
    while (f != NULL && strcmp(f->id, id) != 0) f = f->next;
    if (f == NULL) return -1;
    return f->size;
}

//Funcao para iniciar o codigo
void initCode (QuadList head) {
    QuadList l = head;
    Quad q;
    instructionFormat3(ldi, $sp, sploc, NULL);
    instructionFormat3(ldi, $gp, gploc, NULL);
    instructionFormat3(ldi, $ra, raloc, NULL);
    InserirFunc("Global");
    
}

//Cria a funcao assembly
void createInstr (QuadList l){
    Quad q;
    Address a1, a2, a3;
    int aux;
    VarKind v;
    while(l != NULL){
        q= l->quad;
        a1 = q.addr1;
        a2 = q.addr2;
        a3 = q.opRes;
        FunList g = funcHeadList;
        switch (q.op){
            case opADD: //Op +
                // CORREÇÃO: a3 (destino) vem primeiro no formato correto: add $destino, $fonte1, $fonte2
                instructionFormat1(add, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opSUB: //Op -
                instructionFormat1(sub, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;
            
            case opMULT: //Op *
                instructionFormat1(mult, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opDIV: //Op /
                instructionFormat1(divi, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opLT: //Op <
                instructionFormat1(slt, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opEQUAL: //Op == (igualdade)
                // Para a == b, usamos: se (a <= b) E (b <= a) então a == b
                // Como temos apenas um registrador de saída, implementamos como:
                // $t_dest = 1 se a == b, 0 caso contrário
                // Usaremos a subtração: se (a - b) == 0, então eles são iguais
                // Para simplificar: usamos sle que faz a <= b
                // Se a <= b E b <= a, então a == b
                // Implementação: usamos sge (a >= b) pois sge = NOT(a < b)
                instructionFormat1(sge, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opLEQUAL: //Op ==
                instructionFormat1(sle, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opGT: //Op >
                instructionFormat1(sgt, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opGREQUAL: //Op >=
                instructionFormat1(sge, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opAND: //Op &
                instructionFormat1(and, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opOR: //Op OR
                instructionFormat1(or, getReg(a3.contents.var.name), getReg(a1.contents.var.name), getReg(a2.contents.var.name));
                break;

            case opASSIGN:
                //printf("entrou no op assign \n");
                instructionFormat2(move, getReg(a1.contents.var.name), getReg(a2.contents.var.name), 0, NULL);
                break;
            
            case opALLOC:
                //printf("entrou no alloc, a3:%d \n",a3.contents.val);
                if (a3.contents.val == 1){
                 //printf("entrou no alloc:%s \n",a2.contents.var.name);
                 inserirVar(a2.contents.var.name, a1.contents.var.name,  a3.contents.val, simple);
                }
                else inserirVar(a2.contents.var.name, a1.contents.var.name,  a3.contents.val, vector);
              //  printf("saiu do alloc \n");
                break;

            case opIMMED:
               // printf("entrou no imed \n");
                instructionFormat3(ldi, getReg(a1.contents.var.name), a2.contents.val, NULL);

                break;

            case opLOAD:

              //printf("a2.contents.var.scope: %s \n", a2.contents.var.scope);
                aux = getVarMemLoc(a2.contents.var.name, a2.contents.var.scope);
             // printf("%d \n", aux);
                if (aux == -1) {
                    v = getVarKind(a2.contents.var.name, "Global");
                    aux = getVarMemLoc(a2.contents.var.name, "Global");
                    if (v == vector) {
                        instructionFormat2(addi, getReg(a1.contents.var.name), $gp, aux, NULL);
                    }
                    else {
                        instructionFormat2(load, getReg(a1.contents.var.name), $gp, aux, NULL);
                    }
                }
                else{

                    v = getVarKind(a2.contents.var.name, a2.contents.var.scope);
                    if (v == vector) {

                        instructionFormat2(addi, getReg(a1.contents.var.name), $sp, aux, NULL);
                    }
                    else {
                        instructionFormat2(load, getReg(a1.contents.var.name), $sp, aux, NULL);
                    }
                }
                break;

            case opSTORE:
                //printf("entrou no STORE \n");
                   aux = getVarMemLoc(a1.contents.var.name, a1.contents.var.scope);
                if (aux == -1) {
                    aux = getVarMemLoc(a1.contents.var.name, "Global");
                    if (a3.kind == String) instructionFormat2(str, getReg(a2.contents.var.name), getReg(a1.contents.var.name), aux, NULL);
                    else instructionFormat2(str, getReg(a2.contents.var.name), $gp, aux, NULL);
                }
                else{
                    if (a3.kind == String) instructionFormat2(str, getReg(a2.contents.var.name), getReg(a1.contents.var.name), aux, NULL);
                    else instructionFormat2(str, getReg(a2.contents.var.name), $sp, aux, NULL);
                }
                break;

            case opVEC:
                v = getVarKind(a2.contents.var.name, a2.contents.var.scope);  //a2.contents.var.scope);
                if (v == simple) v = getVarKind(a2.contents.var.name, "Global");
                aux = getVarMemLoc(a2.contents.var.name, a2.contents.var.scope);// a2.contents.var.scope);
                if (v == vector) {
                    if (aux == -1) {
                        aux = getVarMemLoc(a2.contents.var.name, "Global");
                        instructionFormat1(add, getReg(a3.contents.var.name), getReg(a3.contents.var.name), $gp);
                    }
                    else{
                        instructionFormat1(add, getReg(a3.contents.var.name), getReg(a3.contents.var.name), $sp);
                    }
                    instructionFormat2(load, getReg(a1.contents.var.name), getReg(a3.contents.var.name), aux, NULL);
                }
                else {
                    instructionFormat2(load, getReg(a1.contents.var.name), $sp, aux, NULL);
                    instructionFormat1(add, getReg(a3.contents.var.name), getReg(a3.contents.var.name), getReg(a1.contents.var.name));
                    instructionFormat2(load, getReg(a1.contents.var.name), getReg(a3.contents.var.name), 0, NULL);
                }
                //printf("saiu do vec \n");
                break;

            case opGOTO:
            //printf("entrou no goto \n");
                instructionFormat4(jmp, -1, a1.contents.var.name);
                break;

            case opIFF: //Op IF
                //printf("entrou no iffalse \n");
                instructionFormat2(beq, getReg(a1.contents.var.name), $zero, -1, a2.contents.var.name);
                break;

            case opRET:
                //printf("entrou no return \n");
                if (a1.kind == String) instructionFormat2(move, $ret, getReg(a1.contents.var.name), 0, NULL);
                instructionFormat2(addi, $ra, $ra, -1, NULL);
                instructionFormat2(load, $jmp, $ra, 0, NULL);
                instructionFormat3(jr, $jmp, 0, NULL);
                break;

            case opFUN:
                if (jmpMain == 0) {
                    //printf("entrou no opFUN \n");
                    instructionFormat4(jmp, -1, "main");
                    jmpMain = 1;
                }
                inserirLabel(a1.contents.var.name);
                InserirFunc(a1.contents.var.name);
                currentArg = 0;
                break;

            case opEND:
                if (strcmp(a1.contents.var.name, "main") == 0) {
                    instructionFormat4(jmp, -1, "end");
                }
                break;

            case opPARAM:
                //printf("entrou no param \n");
                instructionFormat2(move, getParamReg(), getReg(a1.contents.var.name), 0, NULL);
                currentParam++;
                break;

            case opCALL:
                //printf("entrou na call \n");
                //printf("teste %s \n", a1.contents.var.name);
                if (strcmp(a1.contents.var.name, "input") == 0) {
                    //printf("entrou AQUI! \n");
                    instructionFormat3(in, getReg(a3.contents.var.name), 0, NULL);
                }
                else if (strcmp(a1.contents.var.name, "output") == 0) {
                    //printf("entrou AlI! \n");
                    instructionFormat2(move, getReg(a3.contents.var.name), getArgReg(), 0, NULL);
                    instructionFormat3(out, getReg(a3.contents.var.name), 0, NULL);
                    instructionFormat4(nop, 0, NULL);
                }
                else {
                    instructionFormat3(ldi, $jmp, getAdjustedLineno(line + 4), NULL);
                    instructionFormat2(str, $jmp, $ra, 0, NULL);
                    instructionFormat2(addi, $ra, $ra, 1, NULL);
                    instructionFormat4(jmp, -1, a1.contents.var.name);
                    instructionFormat2(move, getReg(a3.contents.var.name), $ret, 0, NULL);
                }
                nArg = a2.contents.val;
                currentParam = 0;
                break;

            case opARG:
                //printf("entrou no opARG \n");
                //printf("Arg: %d", currentArg);
                inserirVar(a3.contents.var.name, a1.contents.var.name, 1, checkType(l));
                //printf("saiu do InserirVar \n");
                FunList f = funcHeadList;
                //printf("Instruct novo\n");
                // CORREÇÃO: Busca na função atual (a3.contents.var.name) ao invés de f->next->id
                int varLoc = getVarMemLoc(a1.contents.var.name, a3.contents.var.name);
                //printf("VarLoc para %s no escopo %s: %d\n", a1.contents.var.name, a3.contents.var.name, varLoc);
                instructionFormat2(str, getArgReg(), $sp, varLoc, NULL);
                currentArg ++;
                break;

            case opLAB:
                //printf("entrou no label \n");
                inserirLabel(a1.contents.var.name);
                break;

            case opHLT: //Finaliza
                inserirLabel("end");
                instructionFormat4(halt, 0, NULL);
                break;

            default:
                instructionFormat4(nop, 0, NULL);
                break;
        }

        l = l->next;
    }
}

//Cria uma nova instrucao
void criaInstruct (QuadList head) {
    QuadList l = head;
    createInstr(l);
    AssemblyCode a = headcode;
    while(a != NULL){
        if(a->kind == instr){
            if(a->line.instruction.opcode == jmp || a->line.instruction.opcode == beq)
                a->line.instruction.im = getLabelLine(a->line.instruction.imlbl);
        }
        a = a->next;
    }
}

//Imprime no terminal as instrucoes em assembly
void printAssembly() {
    AssemblyCode a = headcode;
    printf("\nC - Assembly Code\n");
    while (a != NULL){
        if(a->kind == instr){ //Eh uma instrucao
            if(a->line.instruction.format == format1){ //Imprime o formato 1
                printf("%d:\t%s %s, %s, %s\n", getAdjustedLineno(a->lineno), NomedeInstr[a->line.instruction.opcode], NomedeReg[a->line.instruction.reg1],
                        NomedeReg[a->line.instruction.reg2], NomedeReg[a->line.instruction.reg3]);
            }
            else if (a->line.instruction.format == format2) {//Imprime o formato 2
                if (a->line.instruction.opcode == move)
                    printf("%d:\t%s %s, %s\n",    getAdjustedLineno(a->lineno), NomedeInstr[a->line.instruction.opcode], NomedeReg[a->line.instruction.reg1],
                                                NomedeReg[a->line.instruction.reg2]);
                else
                    printf("%d:\t%s %s, %s, %d\n",    getAdjustedLineno(a->lineno), NomedeInstr[a->line.instruction.opcode], NomedeReg[a->line.instruction.reg1],
                                                NomedeReg[a->line.instruction.reg2], a->line.instruction.im);

            }
            else if (a->line.instruction.format == format3) {//Imprime o formato 3
                if (a->line.instruction.opcode == jr || a->line.instruction.opcode == in || a->line.instruction.opcode == out)
                    printf("%d:\t%s %s\n",    getAdjustedLineno(a->lineno), NomedeInstr[a->line.instruction.opcode], NomedeReg[a->line.instruction.reg1]);
                else
                    printf("%d:\t%s %s, %d\n",    getAdjustedLineno(a->lineno), NomedeInstr[a->line.instruction.opcode], NomedeReg[a->line.instruction.reg1],
                                                a->line.instruction.im);
            }
            else {
                if (a->line.instruction.opcode == halt || a->line.instruction.opcode == nop)
                    printf("%d:\t%s\n",    getAdjustedLineno(a->lineno), NomedeInstr[a->line.instruction.opcode]);
                else
                    printf("%d:\t%s %d\n",    getAdjustedLineno(a->lineno), NomedeInstr[a->line.instruction.opcode], a->line.instruction.im);
            }
        }
        else {
            printf(".%s\n", a->line.label); //Imprime as labels
        }
        a = a->next;
    }
}

void codeAssembly (QuadList head) {
    initCode(head);
    criaInstruct(head);
    printAssembly();
}

AssemblyCode getAssembly(){
    return headcode;
}

int getSize(){
    return line -1;
}