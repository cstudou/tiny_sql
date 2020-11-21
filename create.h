#pragma once

#include <iostream>
#include <stdio.h>

#include <vector>
#include <unordered_map>
#include "sql_main.h"
using namespace std;

constexpr auto MAXSIZE = 150;


class TableMessage   //建表时各表项信息
{
public:
	char* GetName() { return name; }
	int Get_Message_Type() { return Message_Type; }
	int Get_Message_Length() { return Message_Length; }
	int Get_Message_bind() { return Message_bind; }
	TableMessage* GetNext() { return next; }
	TableMessage* Merge(TableMessage* s1, TableMessage* s2)
	{
		TableMessage* cur = s1;
		while (cur->GetNext())
		{
			cur = cur->GetNext();
		}
		cur->SetNext(s2);
		return s1;
	}
	void SetNext(TableMessage* n) { this->next = n; }
	void unit(char* s1, int s2, int s3, int s4)
	{
		this->name = s1, this->Message_Type = s2, this->Message_Length = s3;
		this->Message_bind = s4;
		this->next = NULL;
	}
private:
	char* name;
	int Message_bind;
	int Message_Type;
	int Message_Length;
	TableMessage* next;
};

class CreateTable
{
public:
	char* GetName() { return this->name; }
	TableMessage* GetNext() { return this->next; }
	void unit(char* s, TableMessage* f)
	{
		this->name = s;
		this->next = f;
	}
private:
	char* name;
	TableMessage* next;

};
struct ColumnNode
{
	char* ColumnName; // 列名
	int type;   //列类型
	int length;  //长度
	vector<char*> ColumnData;  //此列数据

};
struct Where_Require
{
	char* Column;
	int intval;
	int type;
	char* str;
};
struct Table {
	char* name;

	int Length;  // 记录条数
	//int words;

	ColumnNode Cnode[MAXSIZE];
	int res;
	//struct _fieldList** list;
};

extern vector<Where_Require> CnameNode;
TableMessage* Make_Create_Definition(char* s1, int& s2, int& s3, int &s4);
CreateTable* Make_Create_Table(char* s, TableMessage* f);
void SaveName(char* s);
void Do_Create(CreateTable*& s);
void DO_SHOW();