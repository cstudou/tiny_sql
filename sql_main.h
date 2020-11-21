#pragma once
#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <sys/stat.h>

#include "create.h"
#include "select.h"
#include "insert.h"
#include "update.h"
using namespace std;
ssize_t rio_writen(int fd, void *usrbuf, size_t n);
void yyerror(const char* s, ...);
extern string ANS;
extern void scan_string(const char* str);
char *GetUsrBuf();
//bool Judge_Where(ExprNode *root);
//SelectWordList* Make_word_expr_list(char* s);
//SelectNode* Make_select_tree(SelectWordList* s1, ExprNode* s2, SelectWordList* s3);
//SelectWordList* Make_word_list_D(SelectWordList* listp, char* s);


