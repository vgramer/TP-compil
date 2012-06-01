#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#include "tp.h"
#include "tp_y.h"

extern void setError();
extern bool verbose;
extern bool noEval;

int indentation = 1;

void printIndentation(){
	int i=0;
	for(i=0;i<indentation;i++){
			printf("    ");
	}
}

void pprintVar(VarDeclP decl, TreeP tree) {
  if (! verbose) return;
  printf("int %s = ", decl->name);
  pprint(tree);
  printf(";");
  /* utile au cas ou l'evaluation se passerait mal ! */
  fflush(NULL);
}


void pprintValueVar(VarDeclP decl) {
  if (! verbose) return;
  if (! noEval) {
    printf("\n// [Valeur: %d]\n", decl->val);
  } else printf("\n");
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
	printf("if( ");
	pprint(getChild(tree, 0));
	printf(" ) {\n");
	
	/*debut du bloc d'instructions*/
	indentation++;
	pprint(getChild(tree, 1));
	indentation--;
	
	/* y a-t-il un bloc 'else' ? */
	if(tree->nbChildren==3){
		printf("\n");
		printIndentation();
		printf("} else {\n");
		/* debut du bloc else */
		indentation++;
		pprint(getChild(tree, 2));
		indentation--;
	}
	
	/* fin du dernier bloc defini */
	printf("\n");
	printIndentation();
	printf("}");

}

/*
 * concate the string source and stringToConcate in string source. 
 * Memory is automaticly reallocated. 
 * Exit the prog is memory cant be reallocated
 */
void concate(char** source, const char* stringToConcate)
{
	/*add  +1 to \0 */
	*source = (char*)realloc(*source, sizeof(char) * (strlen(*source)  + strlen(stringToConcate) +1) );
	
	if(*source == 0)
	{
		printf("\n\n------------------------\n ERROR CAN NOT REALOCC MEMORY !!!! \n------------------------");
		exit(1);
	}
	
	/*concate the strings*/
	*source =strcat(*source, stringToConcate);
}

/* visits nodes of the given tree to define the format string */
void makeFormatList(TreeP content, char** formatList){
	if(content->op == ARGL){
		/* if arguments list tree found, then recursively call make format string on both children*/
		makeFormatList(getChild(content,0), formatList);
		makeFormatList(getChild(content,1), formatList);
	} 
	else {
		if(content->op == STR)
		{
			/*first realoc mem to have enought space to add str format and add it to list*/
			concate(formatList, "\%s");			
		}
		else if(content->op == ID) /*variable case=> show the value of the variable*/
		{
			/*first realoc mem to have enought space to add str format and add it to list*/
			concate(formatList, "\%d");
		}
		else /* numerical constant case */
		{	
			/*first realoc mem to have enought space to add str format and add it to list*/	
			concate(formatList, "\%d");
		}
	}
}

/* Affichage d'un PUT */
void pprintPUT(TreeP tree) {
  
  /* allocate a buffer for making the format string */
  char* formatList=NULL;
  formatList=calloc(1,sizeof(char));
  
  /*TODO generate a function which print and return 0*/
  makeFormatList(getChild(tree, 0),&formatList);
  
  /* ecriture de la chaine de formattage */
  printIndentation();
  printf("printf(\"%s\",",formatList);
  
  /* ajout des parametres */
  pprint(getChild(tree, 0));
  printf(");");
  
  /* free buffer */
  free(formatList);
}

/* Affichage d'un GET */
void pprintGet() {
  printf("get()");
}

/* Affichage de la boucle pour */
void pprintPour(TreeP tree) {
	
	char* var = getChild(tree, 0)->u.str;
	int begin = getChild(tree, 1)->u.val;
	int end = getChild(tree, 2)->u.val;
	
	char symbol [2] = "<";
	char increment [3]= "++";
	
	if(begin > end)
	{
		symbol[0] = '>';
		increment[0] = '-';
		increment[1] = '-';
	}
	
	/* instruction for en fonction des parametres */
	printIndentation();
	printf("for(int %s = %d; %s %s %d ; %s%s)\n",var, begin, var, symbol, end , var , increment);
	printIndentation();
	printf("{\n");
	
	/* blocs d'instructions*/
	indentation++;
	pprint(getChild(tree, 3));
	indentation--;
	
	/*cloture du bloc d'instructions*/
	printf("\n");
	printIndentation();
	printf("}");
}

/* Affichage de la boucle tantque - faire */
void pprintTantQue(TreeP tree) {

  /* instruction while */
  printIndentation();
  printf("while(");
  pprint(getChild(tree,0));
  printf(")");
  printIndentation();
  printf("{\n");
  
  /* bloc d'instructions */
  indentation++;
  pprint(getChild(tree,1));
  indentation--;
  
  /* fermeture du bloc d'instructions */
  printf("\n");
  printIndentation();
  printf("}");
  
}
/* Affichage de la boucle faire - tantque */
void pprintFaireTantQue(TreeP tree) {
  printIndentation();
  printf("do\n");
  printIndentation();
  printf("{\n");
  indentation++;
  pprint(getChild(tree,0));
  indentation--;
  printf("\n");
  printIndentation();
  printf("}while (");
  pprint(getChild(tree,1));
  printf(");");
}
/* Affichage d'un operateur unaire (not, +) */
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
  case ID:case STR:  case VARDECL:  printf("%s", tree->u.str); break;
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
  case AND:   pprintTree2(tree, " && "); break;
  case OR:    pprintTree2(tree, " || "); break;
  case BINAND:pprintTree2(tree, " & "); break;
  case BINOR: pprintTree2(tree, " | "); break;
  case BINXOR:pprintTree2(tree, " ^ "); break;
  case AFF:   printIndentation(); pprintTree2(tree, " = "); printf(";"); break;
  case INSTRL:pprintTree2(tree, "\n"); break; 
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

void pprintMain(TreeP tree) {
  if (! verbose) return;
  printf("#include <stdio.h>\n#include <stdlib.h>\n\nint main()\n{\n");
  pprint(tree);
  printf("\n");
  printIndentation();
  printf("return EXIT_SUCCESS;\n}\n\n");
  fflush(NULL);
}
