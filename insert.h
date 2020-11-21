#pragma once
#include "sql_main.h"
#include <iostream>
#include <vector>
#include <queue>
#include <stdlib.h>
#include <fstream>
#define MAX_INSERT_COLUMN 150
using namespace std;

class InsertNode
{
public:
	/****************************************/
	void print()
	{
		printf("this is the insert into table %s\n", str);
		printf("this is the insert column \n");
		Name->print();
		printf("this is the insert data ");
		NextExpr->make_print(NextExpr);
	}
	/***********************************/
	void unit(char* name, SelectWordList *p, ExprNode *f)
	{
		this->str = name, this->Name = p, this->NextExpr = f;
    }
	char* GetStr() { return str; }
	SelectWordList * GetName() { return Name; }
	ExprNode * GetNextExpr() { return NextExpr; }

private:
	char* str;
	SelectWordList* Name;
	ExprNode* NextExpr;
	
};
ExprNode* Make_insert_vals(ExprNode* x, ExprNode* y);
ExprNode* Merge_insert_vals(ExprNode* x, ExprNode* y);
InsertNode* Make_insert_tree(char* name, SelectWordList* p, ExprNode* s);   // 表名， 表列， 数据 
void InsertExec(InsertNode*&);

