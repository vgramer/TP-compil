#ifndef __pprintCODE_H__
#define __pprintCODE_H__

#include <unistd.h>
#include <stdio.h>

/*dispatch to the right pprintCode function*/
void pprint(TreeP tree);

/* add space to have an "indentated" code*/
void printIndentation();

/* prefixed pprinterate Code for a binary expression */
void pprintPrefixTree2(TreeP tree, char *op);

/* pprinterate Code for a binary expression:ie call the right pprinteration code for the 2 children*/
void pprintTree2(TreeP tree, char *op);

/* pprinterate Code for if then else statement*/
void pprintIf(TreeP tree);

/* pprinterate code for PUT statement (ie printf) */
void pprintPUT(TreeP tree);

/* Affichage d'un GET */
void pprintGet();

/* pprinterate code for the "for" loop */
void pprintPour(TreeP tree);

/* pprinterate code for the "while" loop */
void pprintTantQue(TreeP tree);

/* pprinterate code for the "do while" loop */
void pprintFaireTantQue(TreeP tree);

/* pprinterate code for  unairy opperation(not, +) */
void pprintUnaire(TreeP tree, char* op);
#endif
