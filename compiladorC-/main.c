#include "globals.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
#define NO_PARSE FALSE
/* set NO_ANALYZE to TRUE to get a parser-only compiler */
#define NO_ANALYZE FALSE

/* set NO_CODE to TRUE to get a compiler that does not
 * generate code
 */
#define NO_CODE FALSE

#include "util.h"
#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"
#if !NO_ANALYZE
#include "analyze.h"
#if !NO_CODE
#include "cgen.h"
#include "assembly.h"
#include "binary.h"
#endif
#endif
#endif

/* allocate global variables */


int lineno = 0;
int iniciolinha = 0;
FILE *source;
FILE *listing;
FILE *code;

/* allocate and set tracing flags */

int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCGen = TRUE;
int TraceCode = FALSE;

int Error = FALSE;

/* Variável global para armazenar o multiplicador de centena */
int centenaMultiplicador = 0;

int main(int argc, char *argv[])
{
  TreeNode *syntaxTree;
  char pgm[120]; /* source code file name */
  if (argc < 2)
  {
    fprintf(stderr, "usage: %s <filename> [centena]\n", argv[0]);
    exit(1);
  }
  
  strcpy(pgm, argv[1]);

    /* Verificar se foi passado o segundo argumento (centena) */
  if (argc >= 3)
  {
    centenaMultiplicador = atoi(argv[2]);
    printf("Contagem iniciará a partir de: %d\n\n", centenaMultiplicador * 100);
  }
  
  if (strchr(pgm, '.') == NULL)
    strcat(pgm, ".cm");
  source = fopen(pgm, "r");

  if (source == NULL)
  {
    fprintf(stderr, "Arquivo %s nao encontrado\n", pgm);
    exit(1);
  }

  listing = stdout; /* send listing to screen */
  fprintf(listing, "\n C - COMPILATION: %s\n", pgm);
#if NO_PARSE
  while (getToken() != ENDFILE)
    ;
#else
  syntaxTree = parse();
  if (TraceParse)
  {
    fprintf(listing, "\nArvore Sintatica:\n");
    printTree(syntaxTree);
  }
#if !NO_ANALYZE
  if (!Error)
  {
    if (TraceAnalyze)
      fprintf(listing, "\nConstruindo a tabela de simbolos\n");
    buildSymtab(syntaxTree);
    if(TraceAnalyze)
      fprintf(listing, "\nTabela de simbolos finalizada\n");
  }
#if !NO_CODE
  if (!Error)
  {
    char *codefile;
    int fnlen = strcspn(pgm, ".");
    codefile = (char *)calloc(fnlen + 4, sizeof(char));
    strncpy(codefile, pgm, fnlen);
    //strcat(codefile, "_binary");
    code = fopen(codefile, "w");
    if (code == NULL)
    {
      printf("Indisponivel para abrir %s\n", codefile);
      exit(1);
    }
    fprintf(listing, "\nConstruindo o codigo intermediario:\n");
    codeGen(syntaxTree, codefile);
    fprintf(listing, "\nCriado o codigo intermediario!\n");
    fprintf(listing, "\nCriando Código Assembly...\n");
    codeAssembly(getIntermediate());
    fprintf(listing,"\nCódigo Assembly criado \n");
    fprintf(listing,"\nCriando o Código Binário ...\n");
    createBinary(getAssembly(), getSize());
    fprintf(listing,"\nCódigo Binário criado\n");
    fclose(code);
  }
#endif
#endif
#endif
  fclose(source);
  return 0;
}
