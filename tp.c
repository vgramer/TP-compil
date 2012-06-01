#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "tp.h"
#include "tp_y.h"

extern int yyparse();
extern int yylineno;

void checkId(TreeP tree, VarDeclP decls);
int eval(TreeP tree, VarDeclP decls);

/* Niveau de 'verbosite'.
 * Par defaut, n'imprime que le resultat et les messages d'erreur
 */
bool verbose = FALSE;

/* Evaluation ou pas. Par defaut, on evalue les expressions */
bool noEval = FALSE;

/* code d'erreur a retourner */
int errorCode = NO_ERROR;

/* Descripteur de fichier pour la lecture des donnees par get */
FILE *fd = NIL(FILE);

/* chaine de caracteres recevant les initialisations de variables pour
 * faciliter la generation de code, et générer un code un peu plus propre */
char* varBuffer=NULL;
/* nom du fichier source ouvert */
char* opened_filename=NULL;

/* Pile des variables localement visibles.
 * Un parametre de fonction ou une variable locale (d'un LET) peut
 * masquer une autre variable ou parametre.
 * Dans une meme portee (var. globale, liste de parametres), on ne doit pas
 * avoir de redeclaration.
 * Enfin, il ne doit pas y avoir d'homonymie entre identificateurs de fonctions
 * et identificateurs de variables. 
 */
VarDeclP currentScope = NIL(VarDecl);


/* Appel:
 *   tp [-option]* programme.txt donnees.dat
 * Le fichier de donnees est obligatoire si le programme execute la
 * construction GET (pas de lecture au clavier), facultatif sinon.
 * Les options doivent apparaitre avant le nom du fichier du programme.
 * Options: -[eE] -[vV] -[hH?]
 */
int main(int argc, char **argv) {
  int fi;
  int i, res;

  for(i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'v': case 'V':
	verbose = TRUE; continue;
      case 'e': case 'E':
	noEval = TRUE; continue;
      case '?': case 'h': case 'H':
	fprintf(stderr, "Appel: tp -e -v programme.txt donnees.dat\n");
	exit(USAGE_ERROR);
      default:
	fprintf(stderr, "Option inconnue: %c\n", argv[i][1]);
	exit(USAGE_ERROR);
      }
    } else break;
  }

  if (i == argc) {
    fprintf(stderr, "Fichier programme manquant\n");
    exit(USAGE_ERROR);
  }

  if ((fi = open(argv[i++], O_RDONLY)) == -1) {
    fprintf(stderr, "erreur: Cannot open source file %s\n", argv[i-1]);
    exit(USAGE_ERROR);
  }

  /* redirige l'entree standard sur le fichier... */
  close(0); dup(fi); close(fi);

  if (i < argc) { /* fichier dans lequel lire les valeurs pour get() */
    if ((fd = fopen(argv[i], "r")) == NULL) {
      fprintf(stderr, "erreur: Cannot open data file %s\n", argv[i]);
      exit(USAGE_ERROR);
    }
  }

  /* Lance l'analyse syntaxique de tout le source, en appelant yylex au fur
   * et a mesure. Execute les actions semantiques en parallele avec les
   * reductions.
   * yyparse renvoie 0 si le source est syntaxiquement correct, une valeur
   * differente de 0 en cas d'erreur syntaxique (eventuellement causee par des
   * erreurs lexicales).
   * Comme l'interpretation globale est automatiquement lancee par les actions
   * associees aux reductions, une fois que yyparse a termine il n'y
   * a plus rien a faire (sauf fermer les fichiers)
   * Si le code du programme contient une erreur, on bloque l'evaluation.
   * S'il n y a que des erreurs contextuelles on essaye de ne pas s'arreter
   * a la premiere mais de continuer l'analyse pour en trouver d'autres, quand
   * c'est possible.
   */
   opened_filename = (char*)calloc(strlen(argv[i-1]),sizeof(char));
   varBuffer = (char*)calloc(1,sizeof(char));
   strcat(opened_filename,argv[i-1]);
  res = yyparse();
  if (fd != NIL(FILE)) fclose(fd);
  return res ? SYNTAX_ERROR : errorCode;
}

char** get_var_buffer() {
	return &varBuffer;
}

const char* get_filename() {
	return opened_filename;
}

void setError(int code) {
  errorCode = code;
  if (code != NO_ERROR) { noEval = TRUE; }
}

/* Lecture dynamique d'une valeur, indiquee par get() dans une expression.
 * Exemple: x := 3 + get(); begin x end
 */
int getValue() {
  int res;
  if (fd == NIL(FILE)) {
    fprintf(stderr, "erreur: Missing data file\n");
    exit(USAGE_ERROR);
  }
  if (fscanf(fd, "%d", &res) == 1) {
    return res;
  } else {
    fprintf(stderr, "erreur: Missing or wrong data in file\n");
    exit(USAGE_ERROR);
  }
} 



/* implemente la fonction put(str, val) du langage */
int putValue(char *str, int val) {
  printf("%s%d\n", str, val);
  return val;
}


/* yyerror:  fonction importee par Bison et a fournir explicitement. Elle
 * est appelee quand Bison detecte une erreur syntaxique.
 * Ici on se contente d'un message minimal.
 */
void yyerror(char *ignore) {
  printf("erreur de syntaxe: Ligne %d\n", yylineno);
}


/* Liberation de la variable : a faire pour de vrai si necessaire ... */
VarDeclP freeVar(VarDeclP decls) {
  VarDeclP old = decls;
  decls = decls->next;
  free(old);
  return decls;
}

/* Verifie qsi besoin ue nouv n'apparait pas deja dans list. l'ajoute en
 * tete et renvoie la nouvelle liste
 */
VarDeclP addToScope(VarDeclP list, VarDeclP nouv, bool noDup) {
  if (noDup) {
    VarDeclP p;
    for(p=list; p != NIL(VarDecl); p = p->next) {
      if (! strcmp(p->name, nouv->name)) {
	fprintf(stderr, "erreur: Double declaration dans la meme portee de %s\n",
		p->name);
	setError(CONTEXT_ERROR); break;
      }
    }
  }
  /* On continue meme en cas de double declaration, pour pouvoir eventuellement
   * detecter plus d'une erreur
   */
  nouv->next=list; return nouv;
}

/* Construit le squelette d'un couple (variable, valeur), sans la valeur.
 * Verifie l'absence d'homonymie avec une fonction.
 */
VarDeclP makeVar(char *name) {
  VarDeclP res = NEW(1, VarDecl);
  res->name = name; res->next = NIL(VarDecl);
  return(res);
}


void declVar(char *name, TreeP tree) {
  VarDeclP pvar = makeVar(name);
  checkId(tree, currentScope);
  pprintVar(pvar, tree, &varBuffer);
  if (! noEval) { pvar->val = eval(tree, currentScope); }
  pprintValueVar(pvar, &varBuffer);
  currentScope = addToScope(currentScope, pvar, TRUE);
}


/* Tronc commun pour la construction d'arbre */
TreeP makeNode(int nbChildren, short op) {
  TreeP tree = NEW(1, Tree);
  tree->op = op;
  tree->nbChildren = nbChildren;
  tree->u.children = nbChildren > 0 ? NEW(nbChildren, TreeP) : NIL(TreeP);
  return(tree);
}


/* Construction d'un arbre a nbChildren branches, passees en parametres */
TreeP makeTree(short op, int nbChildren, ...) {
  va_list args;
  int i;
  TreeP tree = makeNode(nbChildren, op);
  va_start(args, nbChildren);
  for (i = 0; i < nbChildren; i++) { 
    tree->u.children[i] = va_arg(args, TreeP);
  }
  va_end(args);
  return(tree);
}


/* Retourne le rankieme fils d'un arbre (de 0 a n-1) */
TreeP getChild(TreeP tree, int rank) {
  return tree->u.children[rank];
}


/* Constructeur de feuille dont la valeur est une chaine de caracteres
 * (un identificateur de variable ou parametre, la chaine d'un PUT).
 */
TreeP makeLeafStr(short op, char *str) {
  TreeP tree = makeNode(0, op);
  tree->u.str = str;
  return(tree);
}


/* Constructeur de feuille dont la valeur est un entier */
TreeP makeLeafInt(short op, int val) {
  TreeP tree = makeNode(0, op); 
  tree->u.val = val;
  return(tree);
}


/* Evaluation d'une variable */
int evalVar(TreeP tree, VarDeclP decls) {
  char *name = tree->u.str;
  while (decls != NIL(VarDecl)) {
    if (! strcmp(decls->name, name)) return(decls->val);
    decls = decls->next;
  }
  /* ne doit pas arriver si on a verifie les ident. avant l'evaluation */
  fprintf(stderr, "Unexpected: Undeclared variable: %s\n", name);
  exit(UNEXPECTED);
}
int  updateVarValue(char* varname,int value,VarDeclP list){
	VarDeclP parcours = list;
	int trouve=FALSE;
	while(parcours != NULL && !trouve){
		if(! strcmp(parcours->name, varname)){
			trouve=TRUE;
			parcours->val=value;
		}else
			parcours = parcours->next;
	}
	return value;
}
/* Evaluation d'une affectation */
int evalAff(TreeP tree, VarDeclP decls) {
  char *name = getChild(tree,0)->u.str;
  VarDeclP parcours = decls;
  while (parcours != NIL(VarDecl)) {
    if (! strcmp(parcours->name, name)) {
		parcours->val = eval(getChild(tree,1),decls);
		return(parcours->val);
	}
    parcours = parcours->next;
  }
  /* ne doit pas arriver si on a verifie les ident. avant l'evaluation */
  fprintf(stderr, "Unexpected: Undeclared variable: %s\n", name);
  exit(UNEXPECTED);
}

/* Evaluation d'un if then else 
 * le premier fils represente la condition,
 * les deux autres fils correspondent respectivement aux parties then et else.
 * Attention a n'evaluer que la partie necessaire !
 */
int evalIf(TreeP tree, VarDeclP decls) {
	int condition = eval(getChild(tree, 0), decls);
	if (condition != 0) {
		eval(getChild(tree, 1), decls);
	} else {
		if(tree->nbChildren==3){
			eval(getChild(tree, 2), decls);
		}
	}
	return condition;
}

/*evalusation de la boucle 'pour' (reevalue les bornes à chaque iteration)*/
int evalFor(TreeP tree, VarDeclP decls){
	TreeP instr = getChild(tree,3);
	TreeP fin = getChild(tree,2);
	TreeP init = getChild(tree,1);
	TreeP var = getChild(tree,0);
	VarDeclP indice = makeVar(var->u.str);
	decls = addToScope(decls,indice,FALSE);
	int finIterations = eval(fin,decls);
	int debut = eval(init,decls), pas = 1, i=0, last=0;
	int condition = TRUE;
	if(finIterations==debut) condition = FALSE;
	if(debut>finIterations) pas = -1;
	for(i=debut;condition;i+=pas){
		updateVarValue(var->u.str,i,decls);
		last = eval(instr,decls);
		if(pas>0)condition = (i < finIterations);
		else condition = (i > finIterations);
	}

	decls = freeVar(decls);

	return last;
}
/* evaluer l'arbre des instructions tant que l'évaluation de l'arbre des
 * conditions ne retourne pas 0 */
int evalPut(TreeP content, VarDeclP decls){
	if(content->op == ARGL){
		evalPut(getChild(content,0),decls);
		evalPut(getChild(content,1),decls);
	} else {
		if(content->op == STR)
			printf("%s",content->u.str);
		else
			printf("%d",eval(content,decls));
	}
	return 0;
}

/* boucle do while evaluer l'arbre des instructions tant
 * que l'évaluation de l'arbre des
 * conditions ne retourne pas 0 */
int evalDoWhile(TreeP tree, VarDeclP decls){
	int last = 0;
	TreeP instr = getChild(tree,0);
	TreeP cond = getChild(tree,1);
	do{
		last = eval(instr,decls);
	}while(eval(cond, decls));
	
	return last;
}

/* boucle while : evaluer l'arbre des instructions tant
 * que l'évaluation de l'arbre des
 * conditions ne retourne pas 0 */
int evalWhile(TreeP tree, VarDeclP decls){
	int last = 0;
	TreeP instr = getChild(tree,1);
	TreeP cond = getChild(tree,0);
	while(eval(cond, decls)){
		last = eval(instr,decls);
	}
	
	return last;
}
/* Evaluation par parcours recursif de l'arbre representant une expression. 
 * Les valeurs des identificateurs situes aux feuilles de l'arbre sont a
 * rechercher dans la liste decls
 * Attention dans chaque cas, n'evaluer que ce qui doit l'etre et au bon moment
 * selon la semantique de l'operateur (cas du IF, and, or, appels de fonction)
 */
int eval(TreeP tree, VarDeclP decls) {
  switch (tree->op) {
  case ID:
    return evalVar(tree, decls);
  case INSTRL:
	eval(getChild(tree,0),decls);
	return eval(getChild(tree,1),decls);
  case CST:
    return(tree->u.val);
  case EQ:
    return (eval(getChild(tree, 0), decls) == eval(getChild(tree, 1), decls));
  case NE:
    return (eval(getChild(tree, 0), decls) != eval(getChild(tree, 1), decls));
  case GT:
    return (eval(getChild(tree, 0), decls) > eval(getChild(tree, 1), decls));
  case GE:
    return (eval(getChild(tree, 0), decls) >= eval(getChild(tree, 1), decls));
  case LT:
    return (eval(getChild(tree, 0), decls) < eval(getChild(tree, 1), decls));
  case LE:
    return (eval(getChild(tree, 0), decls) <= eval(getChild(tree, 1), decls));
  case ADD:
    return (eval(getChild(tree, 0), decls) + eval(getChild(tree, 1), decls));
  case SUB:
    return (eval(getChild(tree, 0), decls) - eval(getChild(tree, 1), decls));
  case MUL:
    return (eval(getChild(tree, 0), decls) * eval(getChild(tree, 1), decls));
  case DIV:
    { int res, res2;
      /* on evalue les deux operandes pour etre homogene avec les autres cas */
      res = eval(getChild(tree, 0), decls);
      res2 = eval(getChild(tree, 1), decls);
      if (res2 == 0) {
        fprintf(stderr, "erreur: Division par zero\n"); exit(EVAL_ERROR);
      } else { return (res / res2); }
    }
    
  case MINUS:
    return (-eval(getChild(tree, 0), decls));
  
  case PLUS:
    return (eval(getChild(tree, 0), decls));

  case AND:
     /*Les spec du tp nous disent que  l'operateur AND doit evaluer dans un 1er temps l'operand gauche et seulement
       si il est vrai evaluer l'opererand droit.  Nous codons en c donc la spec est respectee.
     */
    return (eval(getChild(tree, 0), decls) && eval(getChild(tree, 1), decls));
  case OR:
    return (eval(getChild(tree, 0), decls) || eval(getChild(tree, 1), decls));
  case NOT:
    return ! eval(getChild(tree, 0), decls) ;
    /* operations bit a bit */
  case BINAND:
    return (eval(getChild(tree, 0), decls) & eval(getChild(tree, 1), decls));
  case BINOR:
    return (eval(getChild(tree, 0), decls) | eval(getChild(tree, 1), decls));
  case BINXOR:
    return (eval(getChild(tree, 0), decls) ^ eval(getChild(tree, 1), decls));
  case IF:
    return (evalIf(tree, decls));
  case AFF:
	return (evalAff(tree,decls));
  case GET:
    return (getValue());
  case PUT:
	evalPut(getChild(tree,0),decls);
    return (printf("\n"));
  case ARGL:
	eval(getChild(tree,0),decls);
	return eval(getChild(tree,1),decls);
  case POUR:
	return (evalFor(tree,decls));
  case FAIRE:
	return (evalDoWhile(tree,decls));
  case TANTQUE:
	return evalWhile(tree,decls);
  default: 
    fprintf(stderr, "erreur #1! Eval unknown operator label(eval tp.c ): %d\n", tree->op);
    exit(UNEXPECTED);
  }
}

int evalMain(TreeP tree) {
 int res;
  /* faire l'impression de l'expression principale avant d'evaluer le resultat,
   *  au cas ou il y aurait une erreur pendant l'evaluation
   */
  pprintMain(tree);
  checkId(tree, currentScope);
  if (noEval) {
    fprintf(stderr, "\nPhase d'evaluation ignoree.\n");
  } else {
      res = eval(tree, currentScope);
      printf("\nresult: %d\n", res);
  }
  return errorCode;
}

void checkIdPour(TreeP tree,VarDeclP decls){
	VarDeclP indice = makeVar(getChild(tree,0)->u.str);
	decls = addToScope(decls,indice,FALSE);
	checkId(getChild(tree,3),decls);
	decls = freeVar(decls);
	return;
}
void checkId(TreeP tree, VarDeclP decls) {
  VarDeclP l; char *name; int i;
  switch (tree->op) {
  case ID:
    name = tree->u.str;
    for(l=decls; l != NIL(VarDecl); l = l->next) {
      if (! strcmp(l->name, name)) { return; }
    }
    fprintf(stderr, "erreur. Variable non declaree: %s\n", name);
    
    setError(CONTEXT_ERROR);
    return;
  case CST:
  case GET:
  case BINAND: case BINOR: case BINXOR:
  case AFF: case INSTRL: case ARGL:
  case TANTQUE: case FAIRE:
  case EQ: case NE: case GT: case GE: case LT: case LE:
  case ADD: case SUB: case MUL: case DIV:case AND:case OR:case NOT:
  case MINUS:case PLUS: case STR:
  case IF:
    for(i=0; i < tree->nbChildren; i++) 
    {
		checkId(getChild(tree, i), decls); 
    }
    return;	
  case POUR:
	checkIdPour(tree,decls);
	return;
  case PUT:
    checkId(getChild(tree, 0), decls);
    return;
  default: 
    fprintf(stderr, "erreur #2! Eval unknown operator label(check id tp.c): %d\n", tree->op);
    exit(UNEXPECTED);
  }
}
