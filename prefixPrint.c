#include "tp.h"
#include "tp_y.h"
#include "prefixPrint.h"

extern void setError();
extern bool Pprint;

/* global variables used for C source code generation */
int indent = 1;

void printIndentation(){
	int i=0;
	for(i=0;i<indent;i++){
			printf("    ");
	}
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
	
	/* instruction if */
	printIndentation();
	printf("SI ");
	pprint(getChild(tree, 0));
	printf(" ALORS\n");

	/*debut du bloc d'instructions*/
	indent++;
	pprint(getChild(tree, 1));
	indent--;
	
	/* y a-t-il un bloc 'else' ? */
	if(tree->nbChildren==3){
		printf("\n");
		printIndentation();
		printf("SINON\n");
		/* debut du bloc else */
		indent++;
		pprint(getChild(tree, 2));
		indent--;
	}
	
	/* fin du dernier bloc defini */
	printf("\n");
	printIndentation();
	printf("FINDESI");

}

/* Affichage d'un PUT */
void pprintPUT(TreeP tree) {
  
  /* ecriture de la chaine de formattage */
  printIndentation();
  printf("ECRIRE(");
  pprint(getChild(tree,0));
  printf(");");

}

/* Affichage d'un GET */
void pprintGet() {
  printf("get()");
}
void pprintAffectation(TreeP tree){
	int old_indent = indent;
	printIndentation();
	pprint(getChild(tree,0));
	printf(" := ");
	indent = 0;
	pprint(getChild(tree,1));
	indent = old_indent;
	if(getChild(tree, 1)->op != AFF)
		printf(";");
}
/* Affichage de la boucle pour */
void pprintPour(TreeP tree) {
  printIndentation();
  printf("POUR %s ALLANT DE %d A %d FAIRE \n",getChild(tree, 0)->u.str,
												getChild(tree, 1)->u.val,
												getChild(tree, 2)->u.val);

  indent ++;
  pprint(getChild(tree, 3));
  indent--;
  
  printf("\n");
  printIndentation();
  printf("FINPOUR");
}

/* Affichage de la boucle tantque - faire */
void pprintTantQue(TreeP tree) {
	
  printIndentation();
  printf("TANQUE ");
  pprint(getChild(tree,0));
  printf(" FAIRE \n");
  indent++;
  pprint(getChild(tree,1));
  indent--;
  printf("\n");
  printIndentation();
  printf("FINTANTQUE");
}

/* Affichage de la boucle faire - tantque */
void pprintFaireTantQue(TreeP tree) {
  printIndentation();
  printf("FAIRE\n");
  indent++;
  pprint(getChild(tree,0));
  indent--;
  printf("\n");
  printIndentation();
  printf("TANTQUE ");
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
  if (! Pprint ) return;
  if (tree == NIL(Tree)) { 
    printf("Unknown"); return;
  }
  switch (tree->op) {
  case ID:case STR:  case VARDECL:  printf("%s", tree->u.str); break;
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
  case AFF:  
	pprintAffectation(tree);
	break;
  case PAR:
    printf("(");
    pprint(getChild(tree, 0));
    printf(")");
    break;
  case INSTRL:pprintTree2(tree, " \n "); break; 
  case ARGL:  pprintTree2(tree, ", "); break;
  case PUT:   pprintPUT(tree);break;
  case GET:   pprintGet(); break;
  case NOT:   pprintUnaire(tree, " !"); break;
  case PLUS:  pprintUnaire(tree, " +"); break;
  case MINUS: pprintUnaire(tree, " -"); break; 
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

