#ifndef __TP_H__
#define __TP_H__

#include <stdarg.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



/* deux macros pratiques, utilisees dans les allocations */
#define NEW(howmany, type) (type *) calloc((unsigned) howmany, sizeof(type))
#define NIL(type) (type *) 0

#define TRUE 1
#define FALSE 0 

int getValue();

typedef int bool;

/* la structure d'un arbre (noeud ou feuille) */
typedef struct _Tree {
  short op;         /* etiquette de l'operateur courant */
  short nbChildren; /* nombre de sous-arbres */
  union {
    char *str;      /* valeur de la feuille si op = ID ou STR */
    int val;        /* valeur de la feuille si op = CST */
    struct _Tree **children; /* tableau des sous-arbres */
  } u;
} Tree, *TreeP;

/* la structure ci-dessous permet de memoriser des listes variable/valeur
 * (entiere). On va construire des listes de la forme { (x 5), (y, 27) } au fur
 * et a mesure qu'on interprete les declarations dans le programme.
 */
typedef struct _Decl
{ char *name;
  int val;
  struct _Decl *next;
} VarDecl, *VarDeclP;



/* Etiquettes additionnelles pour les arbres de syntaxe abstraite. Les tokens
 * tels que ADD, SUB, etc. servent directement d'etiquette.
 * Attention donc a ne pas donner des valeurs identiques a celles des tokens.
 */
#define NE 1
#define EQ 2
#define LT 3
#define LE 4
#define GT 5
#define GE 6
#define CALL 7
#define MINUS 8
#define PLUS 9



/* Codes d'erreurs */
#define NO_ERROR	0
#define USAGE_ERROR	1
#define LEXICAL_ERROR	2
#define SYNTAX_ERROR    3
#define CONTEXT_ERROR	4
#define EVAL_ERROR	5
#define UNEXPECTED	10


/* Type pour la valeur de retour de Flex et les actions de Bison
 * le premier champ est necessaire pour flex
 * les autres correspondent aux variantes utilisees dans les actions
 * associees aux productions de la grammaire. 
*/
typedef union
{ char C;
  char *S;   
  int I;    
  VarDeclP D;
  TreeP T;
} YYSTYPE;


/* construction des declarations */
VarDeclP makeVar(char *name);
VarDeclP addToScope(VarDeclP list, VarDeclP nouv, bool noDup);
void declVar(char *name, TreeP tree);

/* construction et accesseur pour les arbres */
TreeP makeLeafStr(short op, char *str);
TreeP makeLeafInt(short op, int val);
TreeP makeTree(short op, int nbChildren, ...);
TreeP getChild(TreeP tree, int rank);

/* evaluateur de l'expression principale */
int evalMain(TreeP tree);

char** get_var_buffer();
const char* get_filename();

#define YYSTYPE YYSTYPE

#endif
