/*
 * Un petit programme de demonstration qui n'utilise que l'analyse lexicale.
 * Permet principalement de tester la correction de l'analyseur lexical et de
 * l'interface entre celui-ci et le programme qui l'appelle.
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"

extern char *strdup(const char *);

/* Fonction appelee par le programme principal pour obtenir l'unite lexicale
 * suivante. Elle est produite par Flex (fichier tp_l.c)
 */
extern int yylex (void);

/* La chaine de caractere en cours de lecture : definie et mise a jour
 * dans tp_l.c
 */
extern char *yytext;

/* Le numero de ligne courant : defini et mis a jour dans tp_l.c */
extern int yylineno;

typedef struct {
  int line;
  char *id;
} IdentS, *IdentP;


/* Variable pour interfacer flex avec le programme qui l'appelle, notamment
 * pour transmettre de l'information, en plus du type d'unite reconnue.
 * Le type YYSTYPE est defini dans tp.h.
 */
YYSTYPE yylval;

bool verbose = FALSE;

/* Table des identificateurs avec leur numero de ligne */
IdentP idents = NIL(IdentS);

/* Compteurs */
int nbIdent = 0;

/* Recherche un identificateur dans la table */
IdentP getIdent(char * id) {
  int i;
  for (i = 0; i < nbIdent; i++) {
    if (!strcmp(id, idents[i].id)) return(&idents[i]);
  }
  return NIL(IdentS);
}

/* Cree un identificateur */
IdentP makeIdent(int line, char *id) {
  IdentP ident = getIdent(id);
  if (!ident) {
    ident = &idents[nbIdent++];
    ident->line = line;
    /* Si on a duplique la chaine dans la partie Flex, il n'y a pas de raison
     * de le faire a nouveau ici.
     * Comme ca risque de dependre de ce que les uns et les autres font dans
     * tp.l, on prefere le (re-)faire ici.
     */
    ident->id = strdup(id);
  }
  return(ident);
}

/* Appel:
 *   tp [-option]* programme.txt
 * Les options doivent apparaitre avant le nom du fichier du programme.
 * Options: -[vV] -[hH?]
 */

int main(int argc, char **argv) {
  idents = NEW(1000, IdentS); /* liste des identificateurs */
  int fi;
  int token;
  int i;

  for(i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'v': case 'V':
	verbose = TRUE; continue;
      case '?': case 'h': case 'H':
	fprintf(stderr, "Appel: tp -v programme.txt\n");
	exit(1);
      default:
	fprintf(stderr, "Option inconnue: %c\n", argv[i][1]);
	exit(1);
      }
    } else break;
  }

  if (i == argc) {
    fprintf(stderr, "Erreur: Fichier programme manquant\n");
    exit(1);
  }

  if ((fi = open(argv[i++], O_RDONLY)) == -1) {
    fprintf(stderr, "Erreur: fichier inaccessible %s\n", argv[i-1]);
    exit(1);
  }

  /* redirige l'entree standard sur le fichier... */
  close(0); dup(fi); close(fi);

  while (1) {
    token = yylex();
    switch (token) {
      /* ATTENTION: au fur et a mesure que vous ajoutez des tokens,
       * il faut etendre la liste des 'case' ci-dessous sinon le programme
       * de test continuera a dire qu'ils ne sont pas reconnus !
       *
       * Le programme de test ne connaissant pas la liste des tokesn que
       * vous allez ajouter, il ne peut pas etre exhaustif !
       */
    case 0:
      printf("Fin de l'analyse lexicale\n");
      printf("Liste des identificateurs avec leur premiere occurrence:\n");
      for (i = 0; i < nbIdent; i++) {
        printf("ligne %d : %s\n", idents[i].line, idents[i].id);
      }
      printf("Nombre d'identificateurs: %4d\n", nbIdent);
      return 0;
    case ID:
      makeIdent(yylineno, yylval.S);
      if (verbose) printf("Identificateur:\t\t%s\n", yylval.S);
      break;
    case CST:
      if (verbose) printf("Constante:\t\t%d\n", yylval.I);
      break;
    case STR:
      if (verbose) printf("Chaine:\t\t\t'%s'\n", yylval.S);
      break;
    case BEG: 
    case END:
    case PUT:
    case GET:
    case IF:
    case THEN:
    case ELSE:
    case ENDIF:
    case AND:
    case OR:
    case NOT:
	case BINAND:
	case BINOR:
	case BINXOR:
    case POUR:
	case ALLANTDE:
	case JUSQUA:
	case FAIRE:
	case FINPOUR:
	case TANTQUE:
	case FINTANTQUE:
      if (verbose) printf("Mot-clef:\t\t%s\n",  yytext);
      break;

    case AFF:
    case ';':
    case ',':
    case '(':
    case ')':
      if (verbose) printf("Symbole:\t\t%s\n",  yytext);
      break;
    case ADD:case SUB:case MUL:case DIV:	
      if (verbose) printf("Oper arithmetique:\t%s\n", yytext);
      break;
    case VARDECL:
	  if (verbose) printf("Operateur de declaration:\t%s\n", yytext);
      break;
    case RELOP:
      /* inutilement complique ici, mais sert a illustrer la difference
       * entre le token et l'information supplementaire qu'on peut associer
       * via la valeur.
       */
      if (verbose) { 
	printf("Oper de comparaison:\t%s ", yytext);
	switch(yylval.C) {
	case EQ: printf("egalite\n"); break;
	case NE: printf("non egalite\n"); break;
	case GT: printf("superieur strict\n"); break;
	case GE: printf("superieur ou egal\n"); break;
	case LT: printf("inferieur strict\n"); break;
	case LE: printf("inferieur ou egal\n"); break;
	default: printf("Unexpected code: %d\n", yylval.C);
	}
      }
      break;
    default:
      printf("Token non reconnu pzt le programme de test(a definir dans tp_lex):\t\"%c\"(%d)\n", token, token);
    }
  }
}
