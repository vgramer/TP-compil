#include <unistd.h>
#include <stdio.h>
#include "tp.h"
#include "tp_y.h"

extern void setError();
extern bool Cgen;
extern bool Eval;

/* global variables used for C source code generation */
int indentation = 1;

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

void printIndentation(char** generated_code){
	int i=0;
	for(i=0;i<indentation;i++){
			concate(generated_code, "    ");
	}
}

void pprintVar(VarDeclP decl, TreeP tree, char** var_decls_buffer) {
  if (! Cgen) return;
  printIndentation(var_decls_buffer);
  concate(var_decls_buffer,"int ");
  concate(var_decls_buffer,decl->name);
  concate(var_decls_buffer," = ");
  pprint(tree,var_decls_buffer);
  concate(var_decls_buffer,";\n");
  /* utile au cas ou l'evaluation se passerait mal ! */
  fflush(NULL);
}


void pprintValueVar(VarDeclP decl, char** generated_code) {
  char value[256] = "";
  if (! Cgen) return;
  if ( Eval) {
    sprintf(value,"%d", decl->val);
    printIndentation(generated_code);
    concate(generated_code, "// [Valeur: ");
    concate(generated_code, value);
    concate(generated_code, "]\n");
  } else printf("\n");
}


/* Affichage d'une expression binaire */
void pprintTree2(TreeP tree, char *op,char** generated_code) {
  pprint(getChild(tree, 0), generated_code);
  concate(generated_code,op);
  pprint(getChild(tree, 1), generated_code);
 }

/* Affichage d'un if then else */
void pprintIf(TreeP tree,char** generated_code) {
	
	/* instruction if */
	printIndentation(generated_code);
	concate(generated_code, "if( ");
	pprint(getChild(tree, 0), generated_code);
	concate(generated_code, " ) {\n");
	
	/*debut du bloc d'instructions*/
	indentation++;
	pprint(getChild(tree, 1),generated_code);
	indentation--;
	
	/* y a-t-il un bloc 'else' ? */
	if(tree->nbChildren==3){
		concate(generated_code,"\n");
		printIndentation(generated_code);
		concate(generated_code,"} else {\n");
		/* debut du bloc else */
		indentation++;
		pprint(getChild(tree, 2),generated_code);
		indentation--;
	}
	
	/* fin du dernier bloc defini */
	concate(generated_code, "\n");
	printIndentation(generated_code);
	concate(generated_code, "}");

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
void pprintPUT(TreeP tree, char** generated_code) {
  
  /* allocate a buffer for making the format string */
  char* formatList=NULL;
  formatList=calloc(1,sizeof(char));
  
  /*TODO generate a function which print and return 0*/
  makeFormatList(getChild(tree, 0),&formatList);
  
  /* ecriture de la chaine de formattage */
  printIndentation(generated_code);
  concate(generated_code,"printf(\"");
  concate(generated_code,formatList);
  concate(generated_code, "\\n\",");
  
  /* ajout des parametres */
  pprint(getChild(tree, 0), generated_code);
  concate(generated_code, ");");
  
  /* free buffer */
  free(formatList);
}

/* Affichage d'un GET */
void pprintGet() {
  printf("get()");
}

/* Affichage de la boucle pour */
void pprintPour(TreeP tree, char** generated_code) {
	
	int int_begin = getChild(tree, 1)->u.val;
	int int_end   = getChild(tree, 2)->u.val;
	
	char* var = getChild(tree, 0)->u.str;
	char begin[256] = "";
	char end  [256] = "";
	
	char symbol [2] = "<";
	char increment [3]= "++";
	
	sprintf(begin,"%d", int_begin);
	sprintf(end,"%d", int_end);
	
	if(int_begin > int_end)
	{
		symbol[0] = '>';
		increment[0] = '-';
		increment[1] = '-';
	}
	
	/* instruction for en fonction des parametres */
	printIndentation(generated_code);
	concate(generated_code, "for(int ");
	concate(generated_code, var);
	concate(generated_code, " = ");
	concate(generated_code, begin);
	concate(generated_code, "; ");
	concate(generated_code, var);
	concate(generated_code, symbol);
	concate(generated_code, end);
	concate(generated_code, "; ");
	concate(generated_code, var);
	concate(generated_code, increment);
	concate(generated_code, ")\n");
	printIndentation(generated_code);
	concate(generated_code, "{\n");
	
	/* blocs d'instructions*/
	indentation++;
	pprint(getChild(tree, 3), generated_code);
	indentation--;
	
	/*cloture du bloc d'instructions*/
	concate(generated_code, "\n");
	printIndentation(generated_code);
	concate(generated_code, "}");
}

/* Affichage de la boucle tantque - faire */
void pprintTantQue(TreeP tree, char** generated_code) {

  /* instruction while */
  printIndentation(generated_code);
  concate(generated_code, "while(");
  pprint(getChild(tree,0), generated_code);
  concate(generated_code, ")");
  printIndentation(generated_code);
  concate(generated_code, "{\n");
  
  /* bloc d'instructions */
  indentation++;
  pprint(getChild(tree,1), generated_code);
  indentation--;
  
  /* fermeture du bloc d'instructions */
  concate(generated_code, "\n");
  printIndentation(generated_code);
  concate(generated_code, "}");
  
}

/* Affichage de la boucle faire - tantque */
void pprintFaireTantQue(TreeP tree, char** generated_code) {
  printIndentation(generated_code);
  concate(generated_code, "do\n");
  printIndentation(generated_code);
  concate(generated_code, "{\n");
  indentation++;
  pprint(getChild(tree,0), generated_code);
  indentation--;
  concate(generated_code, "\n");
  printIndentation(generated_code);
  concate(generated_code, "}while (");
  pprint(getChild(tree,1), generated_code);
  concate(generated_code, ");");
}

/* Affichage d'un operateur unaire (not, +) */
void pprintUnaire(TreeP tree, char* op, char** generated_code) {
  concate(generated_code, "( ");
  concate(generated_code, op);
  concate(generated_code, " ( ");
  pprint(getChild(tree, 0), generated_code);
  concate(generated_code, " ) )");
}
/* Affichage recursif d'un arbre representant une expression. */
void pprint(TreeP tree, char** generated_code) {
	char value[256];
  if (! Cgen ) return;
  if (tree == NIL(Tree)) { 
    printf("Unknown"); return;
  }
  switch (tree->op) {
  case ID:case STR:  case VARDECL:  concate(generated_code, tree->u.str); break;
  case CST:   sprintf(value,"%d", tree->u.val); concate(generated_code, value); break;
  case EQ:    pprintTree2(tree, " = ",generated_code); break;
  case NE:    pprintTree2(tree, " <> ",generated_code); break;
  case GT:    pprintTree2(tree, " > ",generated_code); break;
  case GE:    pprintTree2(tree, " >= ",generated_code); break;
  case LT:    pprintTree2(tree, " < ",generated_code); break;
  case LE:    pprintTree2(tree, " <= ",generated_code); break;
  case ADD:   pprintTree2(tree, " + ",generated_code); break;
  case SUB:   pprintTree2(tree, " - ",generated_code); break;
  case MUL:   pprintTree2(tree, " * ",generated_code); break;
  case DIV:   pprintTree2(tree, " / ",generated_code); break;
  case AND:   pprintTree2(tree, " && ",generated_code); break;
  case OR:    pprintTree2(tree, " || ",generated_code); break;
  case BINAND:pprintTree2(tree, " & ",generated_code); break;
  case BINOR: pprintTree2(tree, " | ",generated_code); break;
  case BINXOR:pprintTree2(tree, " ^ ",generated_code); break;
  case AFF:  
	printIndentation(generated_code);
	pprintTree2(tree, " = ",generated_code);
	concate(generated_code,";");
	break;
  case INSTRL:pprintTree2(tree, "\n",generated_code); break; 
  case ARGL:  pprintTree2(tree, ", ",generated_code); break;
  case PUT:   pprintPUT(tree,generated_code);break;
  case GET:   pprintGet(); break;
  case NOT:   pprintUnaire(tree, " !",generated_code); break;
  case PLUS:  pprintUnaire(tree, " +",generated_code); break;
  case MINUS: pprintUnaire(tree, " -",generated_code); break; 
  case IF:    pprintIf(tree,generated_code); break;
  case POUR:  pprintPour(tree,generated_code); break;
  case TANTQUE:pprintTantQue(tree,generated_code);break;
  case FAIRE: pprintFaireTantQue(tree,generated_code);break;
  
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
  char* generated_code = NULL;
  FILE* output_file = NULL;
  char** VarBuffer = get_var_buffer();
  char output_filename[256] = "";
  sprintf(output_filename,"%s.c",get_filename());

  generated_code = (char*) calloc(1,sizeof(char));
  
  if (! Cgen) return;
  output_file = fopen(output_filename,"w+");
  if(!output_file) {
	printf("ne peut compiler dans le fichier '%s'. verifiez les autorisations.\n", output_filename);
	return;
  }
  else{
	printf("Generation du fichier '%s' ...\n",output_filename);
  }
  fprintf(output_file,"#include <stdio.h>\n#include <stdlib.h>\n\nint main()\n{\n%s\n",*VarBuffer);
  pprint(tree,&generated_code);
  concate(&generated_code, "\n");
  printIndentation(&generated_code);
  fprintf(output_file,"%sreturn EXIT_SUCCESS;\n}\n\n",generated_code);
  fclose(output_file);
  printf("Generation terminee avec succes.\n");
}
