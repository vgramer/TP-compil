#include "tp.h"
#include "tp_y.h"
#include "genCode.h"

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

void genIndentation(char** generated_code){
	int i=0;
	for(i=0;i<indentation;i++){
			concate(generated_code, "    ");
	}
}

void genVar(VarDeclP decl, TreeP tree, char** var_decls_buffer) {
  if (! Cgen) return;
  genIndentation(var_decls_buffer);
  concate(var_decls_buffer,"int ");
  concate(var_decls_buffer,decl->name);
  concate(var_decls_buffer," = ");
  gen(tree,var_decls_buffer);
  concate(var_decls_buffer,";\n");
  /* utile au cas ou l'evaluation se passerait mal ! */
  fflush(NULL);
}


void genValueVar(VarDeclP decl, char** generated_code) {
  char value[256] = "";
  if (! Cgen) return;
  if ( Eval) {
    sprintf(value,"%d", decl->val);
    genIndentation(generated_code);
    concate(generated_code, "// [Valeur: ");
    concate(generated_code, value);
    concate(generated_code, "]\n");
  } else printf("\n");
}


/* Affichage d'une expression binaire */
void genTree2(TreeP tree, char *op,char** generated_code) {
  gen(getChild(tree, 0), generated_code);
  concate(generated_code,op);
  gen(getChild(tree, 1), generated_code);
 }

/* Affichage d'un if then else */
void genIf(TreeP tree,char** generated_code) {
	
	/* instruction if */
	genIndentation(generated_code);
	concate(generated_code, "if( ");
	gen(getChild(tree, 0), generated_code);
	concate(generated_code, " ) {\n");
	
	/*debut du bloc d'instructions*/
	indentation++;
	gen(getChild(tree, 1),generated_code);
	indentation--;
	
	/* y a-t-il un bloc 'else' ? */
	if(tree->nbChildren==3){
		concate(generated_code,"\n");
		genIndentation(generated_code);
		concate(generated_code,"} else {\n");
		/* debut du bloc else */
		indentation++;
		gen(getChild(tree, 2),generated_code);
		indentation--;
	}
	
	/* fin du dernier bloc defini */
	concate(generated_code, "\n");
	genIndentation(generated_code);
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
void genPUT(TreeP tree, char** generated_code) {
  
  /* allocate a buffer for making the format string */
  char* formatList=NULL;
  formatList=calloc(1,sizeof(char));
  
  /*TODO generate a function which print and return 0*/
  makeFormatList(getChild(tree, 0),&formatList);
  
  /* ecriture de la chaine de formattage */
  genIndentation(generated_code);
  concate(generated_code,"printf(\"");
  concate(generated_code,formatList);
  concate(generated_code, "\\n\",");
  
  /* ajout des parametres */
  gen(getChild(tree, 0), generated_code);
  concate(generated_code, ");");
  
  /* free buffer */
  free(formatList);
}

/* Affichage d'un GET */
void genGet(char** generated_code) {
  concate(generated_code, "__ask_int_value()");
}

/* Affichage de la boucle pour */
void genPour(TreeP tree, char** generated_code) {
	
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
	genIndentation(generated_code);
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
	genIndentation(generated_code);
	concate(generated_code, "{\n");
	
	/* blocs d'instructions*/
	indentation++;
	gen(getChild(tree, 3), generated_code);
	indentation--;
	
	/*cloture du bloc d'instructions*/
	concate(generated_code, "\n");
	genIndentation(generated_code);
	concate(generated_code, "}");
}

/* Affichage de la boucle tantque - faire */
void genTantQue(TreeP tree, char** generated_code) {

  /* instruction while */
  genIndentation(generated_code);
  concate(generated_code, "while(");
  gen(getChild(tree,0), generated_code);
  concate(generated_code, ")");
  genIndentation(generated_code);
  concate(generated_code, "{\n");
  
  /* bloc d'instructions */
  indentation++;
  gen(getChild(tree,1), generated_code);
  indentation--;
  
  /* fermeture du bloc d'instructions */
  concate(generated_code, "\n");
  genIndentation(generated_code);
  concate(generated_code, "}");
  
}

/* Affichage de la boucle faire - tantque */
void genFaireTantQue(TreeP tree, char** generated_code) {
  genIndentation(generated_code);
  concate(generated_code, "do\n");
  genIndentation(generated_code);
  concate(generated_code, "{\n");
  indentation++;
  gen(getChild(tree,0), generated_code);
  indentation--;
  concate(generated_code, "\n");
  genIndentation(generated_code);
  concate(generated_code, "}while (");
  gen(getChild(tree,1), generated_code);
  concate(generated_code, ");");
}

/* Affichage d'un operateur unaire (not, +) */
void genUnaire(TreeP tree, char* op, char** generated_code) {
  concate(generated_code, "( ");
  concate(generated_code, op);
  concate(generated_code, " ( ");
  gen(getChild(tree, 0), generated_code);
  concate(generated_code, " ) )");
}
/* Affichage recursif d'un arbre representant une expression. */
void gen(TreeP tree, char** generated_code) {
	char value[256];
  if (! Cgen ) return;
  /*printf("tree->op : %d",tree->op);*/
  if (tree == NIL(Tree)) { 
    printf("Unknown"); return;
  }
  switch (tree->op) {
  case ID:case STR:  case VARDECL:  concate(generated_code, tree->u.str); break;
  case CST:   sprintf(value,"%d", tree->u.val); concate(generated_code, value); break;
  case EQ:    genTree2(tree, " = ",generated_code); break;
  case NE:    genTree2(tree, " <> ",generated_code); break;
  case GT:    genTree2(tree, " > ",generated_code); break;
  case GE:    genTree2(tree, " >= ",generated_code); break;
  case LT:    genTree2(tree, " < ",generated_code); break;
  case LE:    genTree2(tree, " <= ",generated_code); break;
  case ADD:   genTree2(tree, " + ",generated_code); break;
  case SUB:   genTree2(tree, " - ",generated_code); break;
  case MUL:   genTree2(tree, " * ",generated_code); break;
  case DIV:   genTree2(tree, " / ",generated_code); break;
  case AND:   genTree2(tree, " && ",generated_code); break;
  case OR:    genTree2(tree, " || ",generated_code); break;
  case BINAND:genTree2(tree, " & ",generated_code); break;
  case BINOR: genTree2(tree, " | ",generated_code); break;
  case BINXOR:genTree2(tree, " ^ ",generated_code); break;
  case AFF:  
	genIndentation(generated_code);
	genTree2(tree, " = ",generated_code);
	concate(generated_code,";");
	break;
  case PAR:
    concate(generated_code,"(");
    gen(getChild(tree, 0), generated_code);
    concate(generated_code,")");
    break;
  case INSTRL:genTree2(tree, "\n",generated_code); break; 
  case ARGL:  genTree2(tree, ", ",generated_code); break;
  case PUT:   genPUT(tree,generated_code);break;
  case GET:   genGet(generated_code); break;
  case NOT:   genUnaire(tree, " !",generated_code); break;
  case PLUS:  genUnaire(tree, " +",generated_code); break;
  case MINUS: genUnaire(tree, " -",generated_code); break; 
  case IF:    genIf(tree,generated_code); break;
  case POUR:  genPour(tree,generated_code); break;
  case TANTQUE:genTantQue(tree,generated_code);break;
  case FAIRE: genFaireTantQue(tree,generated_code);break;
  
  default:
    /* On signale le probleme mais on ne quitte pas le programme pour autant
     * car ce n'est pas dramatique !
     */
    fprintf(stderr, "Erreur! gen : etiquette d'operator inconnue(print.c): %d\n", 
	    tree->op);
    setError(UNEXPECTED);
  }
}

void genMain(TreeP tree) {
  char* generated_code = NULL;
  FILE* output_file = NULL;
  char** VarBuffer = get_var_buffer();
  char* output_filename = calloc(1,sizeof(char));
  concate(&output_filename,get_filename());concate(&output_filename,".c");

  generated_code = (char*) calloc(1,sizeof(char));
  if (! Cgen) return;
  output_file = fopen(output_filename,"w+");
  if(!output_file) {
	  printf("ne peut compiler dans le fichier '%s'. verifiez les autorisations.\n", output_filename);
	  return;
	}
	printf("\nCompilation du code C dans '%s'...",output_filename);
  fprintf(output_file,"#include <stdio.h>\n#include <stdlib.h>\n\n");
  fprintf(output_file, "int __get_int_value(){\n");
  fprintf(output_file, "\tint value = 0;");
  fprintf(output_file, "\tprintf(\"Veuillez saisir un nombre entier:\");\n");
  fprintf(output_file, "\tscanf(\"%%d\",&value);\n");
  fprintf(output_file, "\treturn value;\n}\n");
  fprintf(output_file, "\nint main()\n{\n%s\n",*VarBuffer);
  gen(tree,&generated_code);
  concate(&generated_code, "\n");
  genIndentation(&generated_code);
  fprintf(output_file,"%sreturn EXIT_SUCCESS;\n}\n\n",generated_code);
  fflush(NULL);
  printf("\nCompilation terminee.\n");
}
