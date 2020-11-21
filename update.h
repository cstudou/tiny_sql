#pragma once
#include <vector>
#include "sql_main.h"
using namespace std;
class UpdateNode
{
public:
	void unit(SelectWordList* a, ExprNode* b, ExprNode* c)
	{
		this->TableName = a, this->WhereNode = c, this->UpdateList = b;
	}
	SelectWordList* GetName() { return TableName; }
	ExprNode* GetWhereNode() { return WhereNode; }
	ExprNode* GetUpdateList() { return UpdateList; }
private:
	SelectWordList* TableName;
	ExprNode* WhereNode;
	ExprNode* UpdateList;
};
UpdateNode* Make_update_node(SelectWordList* n, ExprNode* List, ExprNode* WhereNode);
ExprNode* Merge_update_list(ExprNode* s, char* n, ExprNode* f);
ExprNode* Make_update_list(char* n, ExprNode* s);
void Do_delete(UpdateNode* root);
extern bool Judge_Where(ExprNode *root);
void Do_update(UpdateNode* root);

extern vector<Where_Require> CnameNode;
