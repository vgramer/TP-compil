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


/* Affichage d'une expression binaire */
void pprintTree2(TreeP tree, char *op) {
  printf("(");
  pprint(getChild(tree, 0));
  printf("%s", op);
  pprint(getChild(tree, 1));
  printf(")");
}

/* Affichage d'un if then else */
void pprintIf(TreeP tree) {
  printf("(if ");
  pprint(getChild(tree, 0));
  printf(" then ");
  pprint(getChild(tree, 1));
  printf(" else ");
  pprint(getChild(tree, 2));
  printf(")");
}

/* Affichage d'un PUT */
void pprintPUT(TreeP tree) {
  printf("(put( ");
  pprint(getChild(tree, 0));
  /*printf(" , ");
  pprint(getChild(tree, 1));*/
  printf("))");
}

/* Affichage d'un GET */
void pprintGet() {
  printf("get()");
}

/* Affichage d'un operateur unaire (GET, NOT) */
void pprintUnaire(TreeP tree, char* op) {
  printf("(%s( ", op);
  pprint(getChild(tree, 0));
  printf("))");
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
  case ADD:   pprintTree2(tree, " + "); break;
  case SUB:   pprintTree2(tree, " - "); break;
  case MUL:   pprintTree2(tree, " * "); break;
  case DIV:   pprintTree2(tree, " / "); break;
  case AND:   pprintTree2(tree, " and "); break;
  case OR:    pprintTree2(tree, " or "); break;
  case AFF:   pprintTree2(tree, " := "); break;
  case INSTRL:pprintTree2(tree, " \n "); break; 
  case ARGL:  pprintTree2(tree, ", "); break;
  case PUT:   pprintPUT(tree); break;
  case GET:   pprintGet(); break;
  case NOT:   pprintUnaire(tree, "not"); break;
  case PLUS:  pprintUnaire(tree, "plus"); break;
  case MINUS: pprintUnaire(tree, "minus"); break; 
  case IF:    pprintIf(tree); break;
   
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
