Digit [0-9]
Letter  [a-zA-Z]
LD  ({Letter}|{Digit})
Star \*
NotStar [^*]
NotStarSlash [^*/]
StrChar [^\\"\n]
StrEsc \\.
Delim [ \n\t\r]
Symbol [;,\\(\\)]

%{
#include <string.h>	/* pour strdup */
#include "tp.h"
#include "tp_y.h"

extern char *strdup(const char *s);

extern YYSTYPE yylval;
extern bool noEval;

int keywordOrId() { 
  /* reconnaitre ci-dessous tous les mots-cles du langage
   * les tokens sont a declarer dans tp.y, puis faire 'make' apres pour
   * produire une nouvelle version de tp_y.h qui fait partie des #include
   * ci-dessus
   */
  if (! strcmp(yytext, "DEBUT")) return(BEG);
  if (! strcmp(yytext, "FIN")) return(END);
  if (! strcmp(yytext, "SI")) return(IF);
  if (! strcmp(yytext, "ALORS")) return(THEN);
  if (! strcmp(yytext, "SINON")) return(ELSE);
  if (! strcmp(yytext, "FINDESI")) return(ENDIF);
  if (! strcmp(yytext, "ECRIRE")) return(PUT);
  if (! strcmp(yytext, "POUR")) return(POUR);
  if (! strcmp(yytext, "ALLANTDE")) return(ALLANTDE);
  if (! strcmp(yytext, "A")) return(JUSQUA);
  if (! strcmp(yytext, "FAIRE")) return(FAIRE);
  if (! strcmp(yytext, "FINPOUR")) return(FINPOUR);
  if (! strcmp(yytext, "TANTQUE")) return(TANTQUE);
  if (! strcmp(yytext, "FINTANTQUE")) return(FINTANTQUE);
  if (! strcmp(yytext, "get")) return(GET);
  if (! strcmp(yytext, "VAR")) return(VARDECL);
  /* si ce n'est pas un mot-clef, sauvegarder le texte de l'identificateur
   * et renvoyer le token associe aux identificateurs
   */
  yylval.S = strdup(yytext);
  return(ID);
}


void lexError(char c) {
  fprintf(stderr, "Skipping invalid character: '%c' on line %d\n", c, yylineno);
}

/* les codes symboliques pour les unites lexicales (CST, IF, ADD, ...) sont
 * definis dans tp_y.h et serviront aussi d'etiquettes pour les sommets de
 * l'arbre de syntaxe abstraite qui sera construit par la suite du processus
 * de compilation.
 * 
 * Les "return" qui apparaissent dans les actions associees aux expressions
 * regulieres peuvent renvoyer:
 * - soit le code symbolique d'une U.L.
 * - soit un simple caractere
 * L'eventuelle partie "valeur" de l'U.L. doit etre stockee dans une des
 * variantes de la variable yylval (definie comme etant de type YYSTYPE)
 * afin que les outils compagnons de l'analyseur lexical puisse l'y recuperer.
 * Attention: le contenu de yylval va etre ecrase a chaque appel a l'analyseur
 * lexical et doit donc etre sauvegarde si besoin.
 */
%}

%%
{Delim}		{/*on ne prend pas en compte les espaces...*/}
{Digit}+        { yylval.I = strtol(yytext, NULL, 0); return(CST); }
{Letter}{LD}*   { return keywordOrId(); }

\"({StrChar}|{StrEsc})*\" { yytext[strlen(yytext)-1]='\0';
							yylval.S = strdup(yytext+1);
							return(STR); }

"/*"({NotStar}|{Star}+{NotStarSlash})*{Star}+"/" { /* Commentaire ! */ }
":="            { return(AFF); }
"+"             { return(ADD); }
"*"             { return(MUL); }
"/"             { return(DIV); }
"-"             { return(SUB); }
"="             { /* Voir la liste des etiquettes dans tp.h */
                  yylval.C = EQ; return(RELOP);
                }
"!="            { yylval.C = NE; return(RELOP); }
">"             { yylval.C = GT; return(RELOP); }
">="             { yylval.C = GE; return(RELOP); }
"<"             { yylval.C = LT; return(RELOP); }
"<="             { yylval.C = LE; return(RELOP); }

"|"             { yylval.C = LE; return(BINOR); }
"&"             { yylval.C = LE; return(BINAND); }
"^"             { yylval.C = LE; return(BINXOR); }
"||"             { yylval.C = LE; return(OR); }
"&&"             { yylval.C = LE; return(AND); }
"!"             { yylval.C = LE; return(NOT); }

{Symbol}        { /* Completer la liste des symboles necessaires */
                  return(yytext[0]);
                }
.               { /* '.' represente tout caractere sauf '\n' !
                   * symbole non reconnu : pour l'instant on le transmet.
                   */
                  return(yytext[0]);
                }
%%
