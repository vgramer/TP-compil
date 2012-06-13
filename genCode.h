#ifndef __GENCODE_H__
#define __GENCODE_H__

#include <unistd.h>
#include <stdio.h>



/* Generate code for variable declaration*/
void genVar(VarDeclP decl, TreeP tree, char** var_decls_buffer);

/* Generate code for variable affection*/
void genValueVar(VarDeclP decl, char** generated_code);

/*dispatch to the right genCode function*/
void gen(TreeP tree,char** generated_code);

/*Generate code for include and  main function*/
void genMain(TreeP);

/*
 * concate the string source and stringToConcate in string source. 
 * Memory is automaticly reallocated. 
 * Exit the prog is memory cant be reallocated
 */
void concate(char** source, const char* stringToConcate);

/* add space to have an "indentated" code*/
void genIndentation(char** generated_code);

/* generate Code for a binary expression:ie call the right generation code for the 2 children*/
void genTree2(TreeP tree, char *op,char** generated_code);

/* Generate Code for if then else statement*/
void genIf(TreeP tree,char** generated_code);

/* visits nodes of the given tree to define the format string(use for printf : format is %s for exemple) */
void makeFormatList(TreeP content, char** formatList);

/* generate code for PUT statement (ie printf) */
void genPUT(TreeP tree, char** generated_code);

/* Affichage d'un GET */
void genGet();

/* Generate code for the "for" loop */
void genPour(TreeP tree, char** generated_code);

/* Generate code for the "while" loop */
void genTantQue(TreeP tree, char** generated_code);

/* Generate code for the "do while" loop */
void genFaireTantQue(TreeP tree, char** generated_code);

/* Generate code for  unairy opperation(not, +) */
void genUnaire(TreeP tree, char* op, char** generated_code);
#endif
