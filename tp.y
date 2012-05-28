/* les tokens ici sont ceux supposes etre renvoyes par l'analyseur lexical
 * A adapter par chacun en fonction de ce qu'il a ecrit dans tp.l
 * Bison ecrase le contenu de tp_y.h a partir de la description de la ligne
 * suivante. Ce sont donc ces lignes qu'il faut adapter si besoin, pas tp_y.h !
 */
%token BEG END
%token INSTRL ARGL
%token<D> AFF
%token<S> ID STR VAR
%token<I> CST
%token<C> RELOP
%token IF THEN ELSE ENDIF
%token PUT
%token BINAND
%token BINOR
%token BINXOR
%token AND
%token OR
%token NOT
%token VARDECL
%token POUR
%token ALLANTDE
%token JUSQUA
%token FAIRE
%token FINPOUR
%token TANTQUE
%token FINTANTQUE


/*ici on "declare les non terminaux"*/
%type <T> loop;
%type <T> put;
%type <T> instr;
%type <T> expr;
%type <T> get;
%type <T> ifThen;
%type <T> cond;
%type <T> arg;
%type <T> argL;
%type <T> instrL;


/* indications de precedence (en ordre croissant) et d'associativite. Les
 * operateurs sur une meme ligne (separes par un espace) ont la meme priorite.
 */
%left IF THEN ELSE ENDIF
%left GET
%left ADD SUB
%left MUL DIV
%left AND OR
%left NOT



%{
#include "tp.h"     /* les definition des types et les etiquettes des noeuds */

extern int yylex();	/* fournie par Flex */
extern void yyerror();  /* definie dans tp.c */
%}

%%
/*
on commence par analyser la partie declaration:
	- creer une variable: couple (clef, valeur) et l' ajoute au current scope qui contient la liste de toutes les variables.
	- on creer un AST sur la partie expr
	- on evalue cette AST

puis on traite l'operation l' expression principale 
	- on creer l AST en vérifiant que tous les identificateurs sont declares
	- puis on evalu cette AST (fonction eval dans tp.c)

*/
programme : declLO BEG instrL END {evalMain($3);}
;

/* les declarations de variables etant memorisees dans une variable globale,
 * les productions qui suivent n'ont pas besoin de retourner de valeur.
 */
declLO :
| declL
;

declL : decl
| declL decl
;


/* une declaration de variable ou de fonction, terminee par un ';'. */
decl : var_decl ';'
;

/* creer une variable: couple (clef, valeur) et l' ajoute au current scope qui contient la liste de toutes les variables
la valeur est directement evaluee
si la variable est deja declaree, le programme l'indique
*/
var_decl : VARDECL ID AFF expr {declVar($2, $4);}
;

/* ici on creer des AST 
$$ est le noeud courrant au quel on va ajouter l' AST produite par la regle
$2..n sont les composantes(fils) de l'arbre que nous somme en train de construire, ce sont deja des AST
*/ 
expr : get
| ADD expr {$$ = makeTree(PLUS, 1, $2);}
| SUB expr {$$ = makeTree(MINUS, 1, $2);}
| expr ADD expr {$$ = makeTree(ADD, 2, $1,$3);}
| expr SUB expr {$$ = makeTree(SUB, 2, $1,$3);}
| expr MUL expr {$$ = makeTree(MUL, 2, $1,$3);}
| expr DIV expr {$$ = makeTree(DIV, 2, $1,$3);}
| expr BINXOR expr {$$ = makeTree(BINXOR, 2, $1, $3);}
| expr BINOR  expr {$$ = makeTree(BINOR , 2, $1, $3);}
| expr BINAND expr {$$ = makeTree(BINAND, 2, $1, $3);}
| CST{ $$ = makeLeafInt(CST , $1);}
| ID { $$ = makeLeafStr(ID,$1);}
;

/*une condition*/
cond : expr RELOP expr {$$ =  makeTree($2, 2, $1,$3);}

/*cond AND cond  et pas expr AND expr afin de n'avoir que des expr booleenne
les operateurs boolean ne sont pas definie sur les entiers ex: "not 3" n'a de sens
*/
| cond AND cond {$$ =  makeTree(AND, 2, $1,$3);}
| cond OR cond {$$ =  makeTree(OR, 2, $1,$3);}
| NOT cond {$$ = makeTree(NOT ,1, $2);}
;

ifThen : IF cond THEN expr ELSE expr ENDIF
{$$ =  makeTree(IF, 3, $2,$4, $6);}
| IF cond THEN expr ENDIF
{$$ =  makeTree(IF, 2, $2,$4);}
;

/* regles pour definir les valeurs des parametres */
argL : arg
| argL ',' arg {$$ = makeTree(ARGL,2,$1,$3); }
;

arg : expr
| STR {$$ = makeLeafStr(STR,$1);}
;

/* regle pour l'equivalent du 'printf */
put : PUT '(' argL ')'
{$$ = makeTree(PUT,1,$3);}
;

/* regle pour l'equivalent du 'scanf' */
get: GET '(' ')'
{$$= makeTree(GET , 0);}
;

/* les boucles sont dcrites pas la regle suivante */
loop: POUR ID ALLANTDE expr JUSQUA expr FAIRE instrL FINPOUR { $$ = makeTree(POUR,4,makeLeafStr(ID,$2),$4,$6,$8);}
| FAIRE instrL TANTQUE cond FINTANTQUE { $$ = makeTree(FAIRE,2,$2,$4);}
| TANTQUE cond FAIRE instrL FINTANTQUE { $$ = makeTree(TANTQUE,2,$2,$4);}
;

/* listes d'instructions */
instrL: instr
| instrL instr { $$ = makeTree(INSTRL,2,$1,$2);}
;


/* une declaration de variable ou de fonction, terminee par un ';'. */
instr : ID AFF expr ';' {$$ = makeTree(AFF,2,makeLeafStr(ID,$1),$3);}
| loop
| put ';'
| ifThen
;
