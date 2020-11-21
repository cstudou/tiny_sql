#pragma once
#include <iostream>
#include <cstring>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include "sql_main.h"
#define MAXCHAR 100
using namespace std;
extern ssize_t rio_writen(int fd, void *usrbuf, size_t n);
//vector<Where_Require> CnameNode;     //存储where子句中出现的列明
//vector<char*> AnswerColumn;
static int res = 0;



class ExprNode
{
public:
	ExprNode* GetExprNode_C(int type, char* t);
	ExprNode* GetExprNode_I(int type, int t);
	ExprNode* GetExprNode_LR(ExprNode* l, ExprNode* r, int t);
	ExprNode() :type(0), line_count(1), str(NULL), intval(0), left(NULL), right(NULL), next(NULL) {}
	//ExprNode* GetExprNode_N(ExprNode* s1, ExprNode* s2, int t);
	/********************/
	void make_print(ExprNode* s)
	{
		if (s->GetLeft() == NULL || s->GetRight() == NULL)
		{
			auto cur = s;
			while (cur)
			{
				printf("%d ", cur->GetStr());
				cur = cur->GetNext();
			}
			printf("\n");
			printf("this is the count %d \n", s->GetLineCount());
			return;
		}
		if (s->GetType() == -1)
		{
			make_print(s->GetLeft());
			make_print(s->GetRight());
		}
	}
	void print()
	{
		printf("this is where ");
		printf("%s != ", this->left->str);
		printf("%s", this->right->str);
	}
	/**************/
	void unit(int t, char* f, int num, ExprNode* l, ExprNode* r)
	{
		this->type = t, this->str = f, this->intval = num, this->left = l, this->right = r;
		/*****************/
		//printf("%s ******* %s ***** %d\n", str, f, num);
		/****************/
	}
	void SetNext(ExprNode* y) { this->next = y; }
	ExprNode* GetNext() { return next; }
	ExprNode* GetLeft() { return this->left; }
	ExprNode* GetRight() { return this->right; }
	int GetType() { return type; }
	char* GetStr() { return this->str; }
	int FindIntval() { return this->intval; }
	void SetIntval(int s) { this->intval = s; }
	void SetLineCount() { this->line_count += 1; }
	int GetLineCount() { return line_count; }
private:
	int type, line_count;
	char* str;
	int intval;
	ExprNode* left, * right, * next;
};

class SelectWordList
{
public:
	SelectWordList* GetSelectWordList_D(SelectWordList* listp, char* s);
	SelectWordList* GetSelectWordList(char* s);
	SelectWordList* Next() { return this->next; }
	char* Findname() { return this->name; }
	void SetNext(SelectWordList*& p) { this->next = p; }
	void unit(char* n, SelectWordList* f)
	{
		this->name = n, this->next = f;
	}
	/*****************************/
	void print()
	{
		if (this != NULL)
		{
			//SelectWordList *p = this->next;
			printf("%s  ", this->name);
			SelectWordList* p = this->next;
			while (p)
			{
				printf("%s  ", p->name);
				p = p->next;
			}
			// this = this->Next();
		}
		printf("\n");
	}
	/***********************************/
private:
	char* name;
	SelectWordList* next;
};

class SelectNode
{
public:
	void SelectExec(SelectNode* s);
	SelectNode* SelectParseTree(SelectWordList* s1, ExprNode* s2, SelectWordList* s3);
	char* GetName() { return name; }
	SelectWordList* GetWordList() { return WordList; }
	ExprNode* GetSelectExprTree() { return this->SelectExprTree; }
	/***********************/
	void print()
	{
		printf("this is column: ");
		WordList->print();

		if (SelectExprTree != NULL)
		{
			SelectExprTree->print();

			printf("\n");
		}
		else
		{
			printf("This is where: NULL\n");
		}
		printf("this is table: %s", name);
		//printf(" %s", name);
		// printf("\n");
	}
	/************************/


	SelectNode* unit(SelectWordList* s1, ExprNode* s2, SelectWordList* s3)
	{
		this->WordList = s1;
		this->SelectExprTree = s2;
		while (s3 != NULL)
		{
			this->name = s3->Findname();
			SelectExec(this);
			/*************************/
			//printf("\n--------------------------------------------------------\n");
			/*************************/
			s3 = s3->Next();
		}
	}
private:
	char* name;
	SelectWordList* WordList;
	ExprNode* SelectExprTree;
};

SelectWordList* Make_word_expr_list(char* s);
SelectNode* Make_select_tree(SelectWordList* s1, ExprNode* s2, SelectWordList* s3);
SelectWordList* Make_word_list_D(SelectWordList* listp, char* s);
ExprNode* Make_expr_node_I(int type, int t);
ExprNode* Make_expr_node_C(int type, char* t);
ExprNode* Make_expr_node_N(ExprNode* s1, ExprNode* s2, int t);
ExprNode* Make_expr_node_LR(ExprNode* l, ExprNode* r, int t);
extern bool Judge_Where(ExprNode *root);
extern vector<Where_Require> CnameNode;
