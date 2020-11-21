#include "sql_main.h"
#include "insert.h"
#include <iostream>
using namespace std;
ExprNode* Make_insert_vals(ExprNode* x, ExprNode* y)
{
	auto res = x;
	while (res->GetNext() != NULL)  res = res->GetNext();
	res->SetNext(y);
	x->SetLineCount();
	return x;
}
ExprNode* Merge_insert_vals(ExprNode* x, ExprNode* y)
{
	int t = -1;
	ExprNode* sf = new ExprNode;

	sf->unit(t, NULL, 0, x, y);
	return sf;
}
InsertNode* Make_insert_tree(char* name, SelectWordList* p, ExprNode* s)   // 表名， 表列， 数据 
{
	InsertNode* sf = new InsertNode;
	sf->unit(name, p, s);
	//sf->print();
	return sf;
}
void InsertExec(InsertNode*& root)
{
	//((ExprNode *)root)->make_print((ExprNode *)root);
	int len, type, count = 1, bound, Length[MAX_INSERT_COLUMN] = {}, Column_ans[MAX_INSERT_COLUMN] = {};
	char kl[MAX_INSERT_COLUMN];
	int B[MAX_INSERT_COLUMN]={};
	string d = root->GetStr();
	string v = d + ".config";      //配置文件名
	d = d + ".dat";
	ifstream OutData(d.c_str(), ios::binary);          //打开数据文件
	ifstream OutConfig(v.c_str());
	if (OutData.good() == 0)
	{
		yyerror("打开插入表失败！");
		return;
	}
	if (OutConfig.good() == 0)
	{
		yyerror("打开插入表配置文件失败！");
		return;
	}
	ofstream InData(d.c_str(), ios::in | ios::ate | ios::binary);
	if (InData.good() == 0)
	{
		yyerror("打开插入表失败！");
		return;
	}
	vector<pair<char*, int>> vi(MAX_INSERT_COLUMN, { NULL, 0 });   //存列数据 
	while (OutConfig >> len >> type >> bound)
	{
	
		char* kl = new char(MAXSIZE);    //不申请内存就会指向同一块地址
		OutData.read(kl, len);   //读出表项 
		vi[count].first = kl, vi[count].second = len;     //保存表项
		Length[count] = len + Length[count - 1];
		B[count]=bound;
		count++;
	}
	// 表项不符合出错
	// 数据太多出错
	// 1. 表项不符合
	int cur = 0;
	auto f = root->GetName();
	while (f != NULL)
	{

		int i = 1;
		for (; i < count; i++)   //遍历所有列 
		{
			if (!strcmp(vi[i].first, f->Findname()))  //找到一列 
			{
				Column_ans[cur++] = i;        //匹配到的列在第i行 
				break;
			}

		}
		if (i == count)
		{
			yyerror("INSERT 列错误！");
			return;
		}
		f = f->Next();
	}
	if (root->GetNextExpr()->GetLineCount() >= count )   //插入项过多 
	{
		yyerror("INSERT 列错误！");
		return;
	}
	//遍历取插入项 
	queue<ExprNode*> q;
	if (root->GetNextExpr())
	{
		q.push(root->GetNextExpr());
	}
	int L = 0;
	OutData.seekg(-8, ios::end);
	char li[10] = {};
	OutData.read(li, 8);
	OutData.close();
	L = atoi(li);
	InData.seekp(-8, ios::end);
	while (!q.empty())
	{
		auto r = q.front();
		q.pop();
		if (r->GetType() == -1)
		{
			q.push(r->GetLeft());
			q.push(r->GetRight());
		}
		else    //叶子节点可以输出 
		{
		
			int line = 0;
			bool flag=true;
			auto g=r;
			for (int i = 1; i < count; i++)   //遍历所有列
			{
			 	if (cur != 0 && Column_ans[line] == i)
			 	{
			 		line++;
			 		g = g->GetNext();   //下一个expr 
			 	}
			 	else if (cur == 0)
			 	{
					g = g->GetNext();   //下一个expr
			 	}
			 	else  //插入空 
			 	{
					if(B[i]==2 || B[i]==3)  //违反完整性约束
			 		{
			 			flag=false;
			 			break;
			 		}
			 	}
			 }
			if(flag==false)
			{
				yyerror("违反完整性约束");
				continue;
			}

			for (int i = 1; i < count; i++)   //遍历所有列
			{	
				if (cur != 0 && Column_ans[line] == i)
				{
					line++;
					if (r->GetType() == 1 || r->GetType() == 0)
					{
					//	cout<<"***"<<r->GetStr()<<"***"<<endl;
						InData.write(r->GetStr(), vi[i].second);
					}
					else
					{
						
						char u[10] = {};
						sprintf(u, "%d", r->FindIntval());
					//	cout<<"***"<<u<<"***"<<endl;
						InData.write(u, vi[i].second);
					}
					r = r->GetNext();   //下一个expr 
				}
				else if (cur == 0)
				{
					
					if (r->GetType() == 1 || r->GetType() == 0)
					{
					//	cout<<"***"<<r->GetStr()<<"***"<<endl;
						InData.write(r->GetStr(), vi[i].second);
					}
					else
					{
						char u[10] = {};
						sprintf(u, "%d", r->FindIntval());
					//	cout<<"***"<<u<<"***"<<endl;
						InData.write(u, vi[i].second);
					}
					r = r->GetNext();   //下一个expr
				}
				else  //插入空 
				{
					char u[10] = {};
					InData.write(u, vi[i].second); //插入NULL
				}
			}
			L++; //新增数据 
		}
	}
	char u[10] = {};
	sprintf(u, "%d", L);
	InData.write(u, 8);
	InData.close();
	OutData.close();
	OutConfig.close();
}
