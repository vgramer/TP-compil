#include <unistd.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"

extern void setError();
extern bool verbose;
extern bool noEval;

void pprintVar(VarDeclP decl, TreeP tree) {
  if (! verbose) return;
  printf("%s := ", decl->name);
  pprint(tree);
  /* utile au cas ou l'evaluation se passerait mal ! */
  fflush(NULL);
}


void pprintValueVar(VarDeclP decl) {
  if (! verbose) return;
  if (! noEval) {
    printf(" [Valeur: %d]\n", decl->val);
  } else printf("\n");
}

/* Affichage prefixe d'une expression binaire */
void pprintPrefixTree2(TreeP tree, char *op) {
  printf("(%s", op);
  pprint(getChild(tree, 0));
  printf(" ");
  pprint(getChild(tree, 1));
  printf(")");
}

/* Affichage d'une expression binaire */
void pprintTree2(TreeP tree, char *op) {
  pprint(getChild(tree, 0));
  printf("%s", op);
  pprint(getChild(tree, 1));
}

/* Affichage d'un if then else */
void pprintIf(TreeP tree) {
  printf("SI (");
  pprint(getChild(tree, 0));
  printf(") ALORS \n");
  pprint(getChild(tree, 1));
  if(tree->nbChildren == 3){
	printf("\n SINON \n");
	pprint(getChild(tree, 2));
  }
  printf("\nFINDESI");
}

/* Affichage d'un PUT */
void pprintPUT(TreeP tree) {
  printf("ECRIRE( ");
  pprint(getChild(tree, 0));
  printf(")\n");
}

/* Affichage d'un GET */
void pprintGet() {
  printf("get()");
}

/* Affichage de la boucle pour */
void pprintPour(TreeP tree) {
  printf("POUR %s ALLANT DE %d A %d FAIRE \n",getChild(tree, 0)->u.str,
												getChild(tree, 1)->u.val,
												getChild(tree, 2)->u.val);
  pprint(getChild(tree, 3));
  printf("\nFINPOUR");
}
/* Affichage de la boucle tantque - faire */
void pprintTantQue(TreeP tree) {
  printf("TANQUE ");
  pprint(getChild(tree,0));
  printf(" FAIRE \n");
  pprint(getChild(tree,1));
  printf("\nFINTANTQUE");
}
/* Affichage de la boucle faire - tantque */
void pprintFaireTantQue(TreeP tree) {
  printf("FAIRE\n");
  pprint(getChild(tree,0));
  printf("\nTANTQUE ");
  pprint(getChild(tree,1));
  printf("FINTANTQUE");
}
/* Affichage d'un operateur unaire (GET, NOT) */
void pprintUnaire(TreeP tree, char* op) {
  printf("(");
  pprint(getChild(tree, 0));
  printf("%s)", op);

}
/* Affichage recursif d'un arbre representant une expression. */
void pprint(TreeP tree) {
  if (! verbose ) return;
  if (tree == NIL(Tree)) { 
    printf("Unknown"); return;
  }
  switch (tree->op) {
  case ID:case STR:    printf("%s", tree->u.str); break;
  case CST:   printf("%d", tree->u.val); break;
  case EQ:    pprintTree2(tree, " = "); break;
  case NE:    pprintTree2(tree, " <> "); break;
  case GT:    pprintTree2(tree, " > "); break;
  case GE:    pprintTree2(tree, " >= "); break;
  case LT:    pprintTree2(tree, " < "); break;
  case LE:    pprintTree2(tree, " <= "); break;
  case ADD:   pprintPrefixTree2(tree, " + "); break;
  case SUB:   pprintPrefixTree2(tree, " - "); break;
  case MUL:   pprintPrefixTree2(tree, " * "); break;
  case DIV:   pprintPrefixTree2(tree, " / "); break;
  case AND:   pprintPrefixTree2(tree, " and "); break;
  case OR:    pprintPrefixTree2(tree, " or "); break;
  case BINAND:pprintPrefixTree2(tree, " binand "); break;
  case BINOR: pprintPrefixTree2(tree, " binor "); break;
  case BINXOR:pprintPrefixTree2(tree, " xor "); break;
  case AFF:   pprintTree2(tree, " := "); break;
  case INSTRL:pprintTree2(tree, " \n "); break; 
  case ARGL:  pprintTree2(tree, ", "); break;
  case PUT:   pprintPUT(tree);break;
  case GET:   pprintGet(); break;
  case NOT:   pprintUnaire(tree, "not"); break;
  case PLUS:  pprintUnaire(tree, "plus"); break;
  case MINUS: pprintUnaire(tree, "minus"); break; 
  case IF:    pprintIf(tree); break;
  case POUR:  pprintPour(tree); break;
  case TANTQUE:pprintTantQue(tree);break;
  case FAIRE: pprintFaireTantQue(tree);break;
  
  default:
    /* On signale le probleme mais on ne quitte pas le programme pour autant
     * car ce n'est pas dramatique !
     */
    fprintf(stderr, "Erreur! pprint : etiquette d'operator inconnue(print.c): %d\n", 
	    tree->op);
    setError(UNEXPECTED);
  }
}

void pprintMain(TreeP tree) {
  if (! verbose) return;
  printf("Expression principale : ");
  pprint(tree);
  fflush(NULL);
}
