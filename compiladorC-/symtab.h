/****************************************************/
/* File: symtab.h                                   */
/* Semantic analyzer interface for TINY compiler    */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/


#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"


/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
 void st_insert( char * name, int lineno, int loc, char* scope, DataType dTypes, IDType idTypes );

/* Function st_lookup returns the memory
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name );

/* Procedure printSymTab prints a formatted
 * listing of the symbol table contents
 * to the listing file
 */
void printSymTab(FILE * listing);

void findMain ();
DataType getFunType(char* nome);
#endif
