#include "globals.h"
#include "symtab.h"
#include "code.h"
#include "cgen.h"
#include "assembly.h"

const char * opcodes[] =  { "nop", "halt", "add", "addi", "sub", "mult", "divi", "mod", "and", "or", "not", "xor", "slt", "sgt", "sle", "sge",
                            "shl", "shr", "move", "ldi", "beq", "bne", "jmp", "i", "out", "str", "load", "jr" };

const char * vetBinario[] = {  "00000", "00001", "00010", "00011", "00100", "00101", "00110", "00111", "01000", "01001", "01010", "01011", "01100", "01101", "01110",
                            "01111", "10000", "10001", "10010", "10011", "10100", "10101", "10110", "10111", "11000", "11001", "11010", "11011", "11100", "11101",
                            "11110", "11111" };

                            //nop ,  htl ,      add ,       addi ,    sub,      mult ,     div ,   AND ,     OR   ,   NOT    ,  XOR ,  SLT      ,  SGT ,
                                //SLE ,     SGE ,      SHFL , SHFR ,  move,     loadi  ,    beq ,    bnq    ,   jump  ,  input  ,  output ,  SW ,   LW  , JUMPR
const char * opBinario[] =   { "010100", "010111", "000000", "000001", "000010", "000100", "011001", "011010", "000110", "000111", "000101", "001000", "001001", "011011",
                                "011100", "011101", "001010", "001011", "011000", "001101", "001111", "010000", "010010", "010101", "010110", "001110", "001100", "010011" };

char * getImmediate (int imm, int size){
    int i = 0;
    char * bin = (char *) malloc(size + 2);
    size--;
    for (unsigned bit = 1u << size; bit != 0; bit >>= 1){
        bin[i++] = (imm & bit) ? '1' : '0';
    }
    bin[i] = '\0';
    return bin;
}

char * assemblyToBinary(Instruction i){
    char * bin = (char *) malloc((32+4+2) * sizeof(char));
    if(i.format == format1){
        sprintf(bin, "%s_%s_%s_%s_%s", opBinario[i.opcode], vetBinario[i.reg2], vetBinario[i.reg3], vetBinario[i.reg1], "00000000000");
    }
    else if (i.format == format2){
        if(i.opcode == move){
            sprintf(bin, "%s_%s_%s_%s_%s", opBinario[i.opcode], vetBinario[i.reg2], vetBinario[i.reg1], "00000", "00000000000");
        }
        else if(i.opcode == str || i.opcode == load || i.opcode == addi){
            sprintf(bin, "%s_%s_%s_%s", opBinario[i.opcode], vetBinario[i.reg2], vetBinario[i.reg1], getImmediate(i.im, 16));
        }
        else{
            sprintf(bin, "%s_%s_%s_%s", opBinario[i.opcode], vetBinario[i.reg1], vetBinario[i.reg2], getImmediate(i.im, 16));
        }
    }
    else if (i.format == format3){
        if(i.opcode == ldi){
            sprintf(bin, "%s_%s_%s_%s", opBinario[i.opcode], "00000", vetBinario[i.reg1], getImmediate(i.im, 16));
        }
        else if(i.opcode == in){
            sprintf(bin, "%s_%s_%s_%s_%s", opBinario[i.opcode], "00000", vetBinario[i.reg1], "00000", "00000000000");
        }
        else {
            sprintf(bin, "%s_%s_%s_%s", opBinario[i.opcode], vetBinario[i.reg1], "00000", getImmediate(i.im, 16));
        }
    }
    else{
        sprintf(bin, "%s_%s", opBinario[i.opcode], getImmediate(i.im, 26));
    }
    return bin;
}

//Funcao para imprimir o binario
void createBinary (AssemblyCode head, int size){
    AssemblyCode a = head;
    FILE * c = code;
    char * bin;

    printf("\nC - Codigo Binario\n");

    while(a != NULL){
        if(a->kind == instr){
            printf("mem[%d] = 32'b", getAdjustedLineno(a->lineno));
            bin = assemblyToBinary(a->line.instruction);
            printf("%s;// %s\n", bin, opcodes[a->line.instruction.opcode]);
        }
        else {
            printf("//%s\n", a->line.label);
        }
        a = a->next;
    }    
}