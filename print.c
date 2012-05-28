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
  printf("))\n");
}

/* Affichage d'un GET */
void pprintGet() {
  printf("get()");
}

/* Affichage de la boucle pour */
void pprintPour(TreeP tree) {
  printf("(POUR %s ALLANT DE %d A %d FAIRE (\n",getChild(tree, 0)->u.str,
												getChild(tree, 1)->u.val,
												getChild(tree, 2)->u.val);
  pprint(getChild(tree, 3));
  printf("\n) FIN POUR)");
}
/* Affichage de la boucle tantque - faire */
void pprintTantQue(TreeP tree) {
  printf("(TANQUE ");
  pprint(getChild(tree,0));
  printf(" FAIRE (\n");
  pprint(getChild(tree,1));
  printf("\n) FINTANTQUE)");
}
/* Affichage de la boucle faire - tantque */
void pprintFaireTantQue(TreeP tree) {
  printf("(FAIRE (\n");
  pprint(getChild(tree,0));
  printf("\n) TANTQUE ");
  pprint(getChild(tree,1));
  printf(" FINTANTQUE)");
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
  case BINAND:pprintTree2(tree, " binand "); break;
  case BINOR: pprintTree2(tree, " binor "); break;
  case BINXOR:pprintTree2(tree, " xor "); break;
  case AFF:   pprintTree2(tree, " := "); break;
  case INSTRL:pprintTree2(tree, " \n "); break; 
  case ARGL:  pprintTree2(tree, ", "); break;
  case PUT:   pprintPUT(tree); break;
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
