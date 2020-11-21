//#pragma once
#include "sql_main.h"
extern bool Judge_Where(ExprNode* root);
//#include "Do_select.cpp"
#include "update.h"
//extern bool Judge_Where(ExprNode *root);


bool Judge(ExprNode* root)
{

	int x, y;
	char* p, * q;
	bool flag = false, flag_p = false, flag_q = false;
	if (root == NULL || root->GetType() < 3)
	{
		return true;           //根节点类型不可能是数字、字符串、或者name
	}
	if (root->GetType() >= 2 && root->GetType() <= 8)     //CMP
	{
		switch (root->GetLeft()->GetType())
		{
		case 0:   //左边为name
			if (CnameNode[res].type == 1)      //类型1代表数字
			{
				x = CnameNode[res].intval;
				flag = true;
			}
			else                            //类型0代表字符串
			{
				p = CnameNode[res].str;
            }
			res++;
			break;
		case 1:   //左边为字符串
			p = root->GetLeft()->GetStr();
			break;
		case 2:  //左边为数字
			x = root->GetLeft()->FindIntval();
			break;
		}
		switch (root->GetRight()->GetType())
		{
		case 0:  //右边为name
			if (CnameNode[res].type == 1)      //类型1代表数字
			{
				y = CnameNode[res].intval;
			}
			else                            //类型0代表字符串
			{
				q = CnameNode[res].str;
			}
			res++;
			break;

		case 1:   //右边为字符串
			q = root->GetRight()->GetStr();
			break;
		case 2:  //右边为数字
			y = root->GetRight()->FindIntval();
			break;
		}
	}
	if (root->GetType() == 9 || root->GetType() == 10)   //and 或者 or
	{
		flag_p = Judge(root->GetLeft());
		flag_q = Judge(root->GetRight());
	}
	switch (root->GetType()) //判断根节点类型   = != < > <= >= && || 
	{
	case 3:
		if (flag == true)   //数字
		{
			return (x == y) ? true : false;
		}
		else
		{
			return (strcmp(p, q) == 0) ? true : false;
		}
	case 4:
		if (flag == true)   //数字
		{
			return (x != y) ? true : false;
		}
		else
		{
			return (strcmp(p, q) == 0) ? false : true;
		}


	case 5:
		if (flag == true)   //数字
		{
			return (x < y) ? true : false;
		}
		else
		{
			return (strcmp(p, q) < 0) ? true : false;
		}
	case 6:
		if (flag == true)   //数字
		{
			return (x > y) ? true : false;
		}
		else
		{
			return (strcmp(p, q) > 0) ? true : false;
		}
	case 7:
		return (x <= y) ? true : false;
	case 8:
		return (x >= y) ? true : false;
	case 9:
		return flag_p && flag_q;
	case 10:
		return flag_q || flag_p;
	}
}


ExprNode* Make_update_list(char* n, ExprNode* s)
{
	auto sf = new ExprNode;
    char *u=new char(10);
    strcpy(u, n);
    sf->unit(0, u, 0, s, NULL);
   // printf("this is lie %s\n", sf->GetStr());
    return sf;
}
ExprNode* Merge_update_list(ExprNode* s, char* n, ExprNode* f)
{
	auto sf = new ExprNode, cur = s;
	sf->unit(0, n, 0, f, NULL);
	while (cur->GetNext())
	{
		cur = cur->GetNext();
	}
	cur->SetNext(f);
	return s;
}
UpdateNode* Make_update_node(SelectWordList* n, ExprNode* List, ExprNode* WhereNode)
{
   // printf("this is");
	auto sf = new UpdateNode;
	sf->unit(n, List, WhereNode);
	return sf;
}

void Do_update(UpdateNode* root)
{
    //printf("haha");
	string s = root->GetName()->Findname(), v;
	v = s + ".config", s += ".dat";
	ifstream OutData(s.c_str(), ios::binary);          //打开数据文件
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
	ofstream InData(s.c_str(), ios::in | ios::ate | ios::binary);
	if (InData.good() == 0)
	{
		yyerror("打开插入表失败！");
		return;
	}
	vector<pair<char*, int>> vi(MAX_INSERT_COLUMN, { NULL, 0 });   //存列数据 
	int len, type, bound, count = 1, Length[MAX_INSERT_COLUMN] = {}, Column_ans[MAX_INSERT_COLUMN] = {}, byte = 0;
	int l = 0, B[MAX_INSERT_COLUMN];
	ExprNode* Column_date[MAX_INSERT_COLUMN] = {};
	auto cur = root->GetUpdateList();
	while (OutConfig >> len >> type >> bound)
	{
		char* kl = new char(MAXSIZE);    //不申请内存就会指向同一块地址
		OutData.read(kl, len);   //读出表项 
		vi[count].first = kl, vi[count].second = type;     //保存表项
	    byte+=len;
		if (cur &&!strcmp(kl, cur->GetStr()))
		{
			// 待更新列
			Column_date[l] = cur->GetLeft();
			Column_ans[l++] = count;
			cur = cur->GetNext();
		}
		Length[count] = len + Length[count - 1];
		B[count]=bound;
		count++;
	}
	OutData.seekg(-8, ios::end);    //表文件最后8字节默认存表的行数，insert时修改
	char li[10] = {};
	OutData.read(li, 8);
	int line = atoi(li), k = 1;
	for (int i = 1; i <= line; i++)   //遍历所有行
	{
		for (int j = 0; j < CnameNode.size(); j++)   //判断where子句中的条件
		{
                
			for (k = 1; k < count; k++)//遍历所有属性列
			{
				if (!strcmp(CnameNode[j].Column, vi[k].first))  //找到待判断的属性列
				{
                    
					char c[MAXCHAR];
					CnameNode[j].type = vi[k].second;
					OutData.seekg(byte * i + Length[k - 1], ios::beg);
					OutData.read(c, Length[k] - Length[k - 1]);
                    
		        	if(vi[k].second==1)		CnameNode[j].intval = atoi(c);       //为字符串
                    else CnameNode[j].str=c;    
		//			printf("this is Cname %s\n", CnameNode[j].str);
					break;  //处理完一个表项就break直到这一行所有表项处理完毕
				}
			}
		}
 //     root->GetWhereNode()->print();
        res=0;
       // printf("type is %d, left=%d", root->GetWhereNode()->GetLeft()->GetType(), rr->GetRight()->GetType());
        if (Judge(root->GetWhereNode())) 
		{
        	// update操作
            for (int k = 0; k < l; k++)
			{

				char c[MAXCHAR] = {};
				InData.seekp(byte * i + Length[Column_ans[k] - 1], ios::beg);
                if(Column_date[k]->GetType() == 2)
                {
                    char u[10]={};
                    sprintf(u, "%d", Column_date[k]->FindIntval());
                    InData.write(u, Length[Column_ans[k]]-Length[Column_ans[k]-1]);
                }
                else InData.write(Column_date[k]->GetStr(), Length[Column_ans[k]] - Length[Column_ans[k] - 1]);
			//	printf("%-8s", c);
			}

		}
        
	}
   // CnameNode.clear();
}

void Do_delete(UpdateNode* root)
{

	string s = root->GetName()->Findname(), v;
	v = s + ".config", s += ".dat";
	ifstream OutData(s.c_str(), ios::binary);          //打开数据文件
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
	ofstream InData(s.c_str(), ios::in | ios::ate | ios::binary);
	if (InData.good() == 0)
	{
		yyerror("打开插入表失败！");
		return;
	}
	vector<pair<char*, int>> vi(MAX_INSERT_COLUMN, { NULL, 0 });   //存列数据 
	int len, type, count = 1, bound, Length[MAX_INSERT_COLUMN] = {}, Column_ans[MAX_INSERT_COLUMN] = {}, byte = 0;
	while (OutConfig >> len >> type >> bound)
	{
		char* kl = new char(MAXSIZE);    //不申请内存就会指向同一块地址
		OutData.read(kl, len);   //读出表项 
		vi[count].first = kl, vi[count].second = type;     //保存表项
		byte += len;
		Length[count] = len + Length[count - 1];
		count++;
	}
	OutData.seekg(-8, ios::end);    //表文件最后8字节默认存表的行数，insert时修改
	char li[10] = {};
	OutData.read(li, 8);
	int line = atoi(li), k = 1;
	for (int i = 1; i <= line; i++)   //遍历所有行
	{
		for (int j = 0; j < CnameNode.size(); j++)   //判断where子句中的条件
		{
			for (k = 1; k < count; k++)//遍历所有属性列
			{
				if (!strcmp(CnameNode[j].Column, vi[k].first))  //找到待判断的属性列
				{
					char c[MAXCHAR] = {};
					CnameNode[j].type = vi[k].second;
					OutData.seekg(byte * i + Length[k - 1], ios::beg);
					OutData.read(c, Length[k] - Length[k - 1]);
					if (vi[k].second == 1)		CnameNode[j].intval = atoi(c), cout<<atoi(c)<<endl;       
					else CnameNode[j].str = c;//cout<<c<<"21"<<endl;
					
					break;  //处理完一个表项就break直到这一行所有表项处理完毕
				}
			}
		}
		res = 0;
		if (Judge(root->GetWhereNode())) 
		{
			//update操作
			for (int k = 1; k < count; k++)
			{
				char c[MAXCHAR] = {'$','#','$'};
				InData.seekp(byte * i + Length[k - 1], ios::beg);
				InData.write(c, Length[k] - Length[k - 1]);

			}
		}
	}
	
}