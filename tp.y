/* les tokens ici sont ceux supposes etre renvoyes par l'analyseur lexical
 * A adapter par chacun en fonction de ce qu'il a ecrit dans tp.l
 * Bison ecrase le contenu de tp_y.h a partir de la description de la ligne
 * suivante. Ce sont donc ces lignes qu'il faut adapter si besoin, pas tp_y.h !
 */
%token BEG END
%token<D> AFF
%token<S> ID STR VAR
%token<I> CST
%token<C> RELOP
%token IF THEN ELSE
%token GET
%token AND
%token OR
%token NOT


/*ici on "declare les non terminaux"*/
%type <T> expr;
%type <T> get;
%type <T> ifThen;
%type <T> cond;


/* indications de precedence (en ordre croissant) et d'associativite. Les
 * operateurs sur une meme ligne (separes par un espace) ont la meme priorite.
 */
%left IF THEN ELSE
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
programme : declLO BEG expr END {evalMain($3)}
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
decl : var_decl
;

/* creer une variable: couple (clef, valeur) et l' ajoute au current scope qui contient la liste de toutes les variables
la valeur est directement evaluee
si la variable est deja declaree, le programme l'indique
*/
var_decl : ID AFF expr ';'{declVar($1, $3);}
;

/* ici on creer des AST 
$$ est le noeud courrant au quel on va ajouter l' AST produite par la regle
$2..n sont les composantes(fils) de l'arbre que nous somme en train de construire, ce sont deja des AST
*/ 
expr : ifThen
| get
| ADD expr {$$ = makeTree(PLUS, 1, $2);}
| SUB expr {$$ = makeTree(MINUS, 1, $2);}
| expr ADD expr {$$ = makeTree(ADD, 2, $1,$3);}
| expr SUB expr {$$ = makeTree(SUB, 2, $1,$3);}
| expr MUL expr {$$ = makeTree(MUL, 2, $1,$3);}
| expr DIV expr {$$ = makeTree(DIV, 2, $1,$3);}
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

ifThen : IF cond THEN expr ELSE expr
{$$ =  makeTree(IF, 3, $2,$4, $6);}
;

get: GET '(' ')'
{$$= makeTree(GET , 0);}
;




