#include "sql_main.h"
using namespace std;
extern string ANS;
bool Judge_Where(ExprNode* root)
{

   // printf("1231231");
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
				printf("this is cmp int %dm\n", x);
				flag = true;
			}
			else                            //类型0代表字符串
			{
				p = CnameNode[res].str;
				printf("this is cmp str %s\n", p);
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
				printf("this is cmp str %s\n", q);
			}
			res++;
			break;

		case 1:   //右边为字符串
			q = root->GetRight()->GetStr();
			printf("this is q %s\n", q);
			break;
		case 2:  //右边为数字
			y = root->GetRight()->FindIntval();
			printf("thi is y %d\n",y);
			break;
		}
	}
	if (root->GetType() == 9 || root->GetType() == 10)   //and 或者 or
	{
		printf( " this is first %d\n", root->GetType());
		flag_p = Judge_Where(root->GetLeft());
		flag_q = Judge_Where(root->GetRight());
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

void SelectNode::SelectExec(SelectNode* s)
{
	//s->print();
	ANS="";
	char cmp[MAXSIZE]={'$','#','$'};
	string c = s->GetName();
	string v = c + ".config";
	c += ".dat";
	ifstream Outconfig(v.c_str(), ios::binary);   //打开读入文件
	ifstream Outdata(c.c_str(), ios::binary);   //打开读入文件
	if (Outdata.good() == 0)
	{
		yyerror("找不到表数据文件");
		return;
	}
	if (Outconfig.good() == 0)
	{
		yyerror("找不到表配置文件");
		return;
	}
	vector<pair<char*, int> > vi(MAXSIZE, { NULL,0 });   //存储属性列名
	vector<int> WordListRes(MAXSIZE);
	int type = 0, len = 0, bound, count = 1, Length[MAXSIZE] = {}, byte = 0, l = 0;
	//BUFFANS[pos]='\n';
	ANS += "\n";
	//printf("\n");
	auto w2 = s->GetWordList();
	auto ww = w2;
	//w->print();
	while (Outconfig >> len >> type >> bound)
	{
		w2=ww;
		char* kl = new char(MAXSIZE);    //不申请内存就会指向同一块地址
		Outdata.read(kl, len);
		byte += len;
		//cout<<"this is kl "<<kl<<endl;//<<" this is name "<<w->Findname()<<endl;
		while (w2)
		{
			
			
			if(!strcmp(kl, w2->Findname()))
			{
				WordListRes[l++] = count; break;
			
				break;
			}
			w2 = w2->Next();
			

		}
		vi[count].first = kl, vi[count].second = type;     //保存表项
		Length[count] = len + Length[count - 1];
		count++;
	}
	if (s->WordList == NULL)   //select *
	{
		for (int k = 1; k < count; k++)
		{
			ANS += vi[k].first;
			for(int f=strlen(vi[k].first); f<=16; f++)
			{
				ANS+=" ";
			}
			//printf("%-8s", vi[k].first);
		}
		ANS+="\n---------------------------------------------------\n";
		//printf("\n---------------------------------------------------\n");
		Outdata.seekg(-8, ios::end);    //表文件最后8字节默认存表的行数，insert时修改
		char li[10] = {};
		Outdata.read(li, 8);
		int line = atoi(li), k = 1;
		for (int i = 1; i <= line; i++)   //遍历所有行
		{
			for (int j = 0; j < CnameNode.size(); j++)   //判断where子句中的条件
			{
				for (k = 1; k < count; k++)//遍历所有属性列
				{
					if (!strcmp(CnameNode[j].Column, vi[k].first))  //找到待判断的属性列
					{
						char *c = new char(20);
						CnameNode[j].type = vi[k].second;
						Outdata.seekg(byte * i + Length[k - 1], ios::beg);
						Outdata.read(c, Length[k] - Length[k - 1]);

						if (vi[k].second == 1)  //数字情况
						{
							CnameNode[j].intval = atoi(c);  //为数字
						}
						else
						{
							CnameNode[j].str = c;       //为字符串'
						}
						break;  //处理完一个表项就break直到这一行所有表项处理完毕
					}
				}
			}
			/*   这一行所有待判断条件都已取出    */
			res = 0;     //重置res指针
			bool flag=false;
			if (Judge_Where(s->GetSelectExprTree()))
			{
				flag=true;
				for (int k = 1; k < count; k++)
				{
					char c[MAXCHAR] = {};
					Outdata.seekg(byte * i + Length[k - 1], ios::beg);
					Outdata.read(c, Length[k] - Length[k - 1]);
					if(!strcmp(c, cmp))	
					{
						flag=false;
						continue;
					}
					for(int i=0; i<strlen(c); i++)
					{
						ANS += c[i];
					}
					for(int f=strlen(c); f<=16; f++)
					{
						ANS+=" ";
					}
				}
			}
			if(flag == true)	ANS+="\n";
				//printf("\n");
		}
		
	}
	else
	{
		auto v = s->GetWordList();
		while (v)
		{
			ANS += v->Findname();
			for(int f=strlen(v->Findname()); f<=16; f++)
			{
				ANS+=" ";
			}
			
			v = v->Next();
		}  //输出表列
		ANS+="\n---------------------------------------------------\n";
		Outdata.seekg(-8, ios::end);    //表文件最后8字节默认存表的行数，insert时修改
		char li[10] = {};
		Outdata.read(li, 8);
		int line = atoi(li), k = 1;
		for (int i = 1; i <= line; i++)
		{
			
			for (int j = 0; j < CnameNode.size(); j++)
			{
			
				for (k = 1; k < count; k++)//遍历所有属性列
				{
				
					if (!strcmp(CnameNode[j].Column, vi[k].first))  //找到待判断的属性列
					{
						
						char *c = new char(20);
						CnameNode[j].type = vi[k].second;
						Outdata.seekg(byte * i + Length[k - 1], ios::beg);
						Outdata.read(c, Length[k] - Length[k - 1]);

						if (vi[k].second == 1)  //数字情况
						{
							CnameNode[j].intval = atoi(c);  //为数字
							
						}
						else
						{
							
							CnameNode[j].str = c;       //为字符串
						}
						break;  //处理完一个表项就break直到这一行所有表项处理完毕
					}
				}
			}
			/*   这一行所有待判断条件都已取出    */
			res = 0;     //重置res指针
			bool flag= false;
			//cout<<s->GetSelectExprTree()->GetType();
			if (Judge_Where(s->GetSelectExprTree())) 
			{
				flag=true;
				
				for (int k = 0; k < l; k++)
				{
					
					char c[MAXCHAR] = {};
			
					Outdata.seekg(byte * i + Length[WordListRes[k] - 1], ios::beg);
					Outdata.read(c, Length[WordListRes[k]] - Length[WordListRes[k] - 1]);
					if(!strcmp(c, cmp))	
					{
						flag=false;
						continue;
					}
					for(int i=0; i<strlen(c); i++)
					{
						ANS += c[i];
					}
					for(int f=strlen(c); f<=16; f++)
					{
						ANS+=" ";
					}
				}
			}
			if(flag == true)
				ANS+="\n";
				//printf("\n");
			
		}
	}
	CnameNode.clear();
	ANS+="\n---------------------------------------------------\n";
	vi.clear();
}


SelectWordList* Make_word_expr_list(char* s)
{
	SelectWordList* sf = new SelectWordList;
	sf->unit(s, NULL);
	return sf;
}
SelectNode* Make_select_tree(SelectWordList* s1, ExprNode* s2, SelectWordList* s3)
{
	SelectNode* sf = new SelectNode;
	sf->unit(s1, s2, s3);
	return sf;
}
SelectWordList* Make_word_list_D(SelectWordList* listp, char* s)
{
	SelectWordList* sf = new SelectWordList;
	sf = sf->GetSelectWordList_D(listp, s);
	return sf;
}
ExprNode* Make_expr_node_C(int type, char* t)
{
	ExprNode* sf = new ExprNode;
	sf->unit(type, t, 0, NULL, NULL);
	return sf;
}
ExprNode* Make_expr_node_I(int type, int t)
{
	ExprNode* sf = new ExprNode;
	sf->unit(type, NULL, t, NULL, NULL);
	return sf;
}
ExprNode* Make_expr_node_N(ExprNode* s1, ExprNode* s2, int t)
{
	ExprNode* sf = new ExprNode;
	if (t == 1)	sf->SetIntval(s1->FindIntval() + s2->FindIntval());
	if (t == 2)	sf->SetIntval(s1->FindIntval() - s2->FindIntval());
	if (t == 3)	sf->SetIntval(s1->FindIntval() * s2->FindIntval());
	if (t == 4)	sf->SetIntval(s1->FindIntval() / s2->FindIntval());
	return sf;
}
ExprNode* Make_expr_node_LR(ExprNode* l, ExprNode* r, int t)
{
	
	ExprNode* sf = new ExprNode;
	sf->unit(t, NULL, 0, l, r);
	return sf;
}
SelectWordList* SelectWordList::GetSelectWordList(char* s)
{
	SelectWordList* p = new SelectWordList;
	p->unit(s, NULL);
	return p;
}

ExprNode* ExprNode::GetExprNode_C(int type, char* t)
{
	ExprNode* s = new ExprNode;
	s->unit(type, t, 0, NULL, NULL);
	return s;
}

ExprNode* ExprNode::GetExprNode_LR(ExprNode* l, ExprNode* r, int t)
{
	ExprNode* s = new ExprNode;
	s->unit(t, NULL, 0, l, r);
	return s;
}

SelectNode* SelectNode::SelectParseTree(SelectWordList* s1, ExprNode* s2, SelectWordList* s3)
{
	SelectNode* s = new SelectNode;
	s->unit(s1, s2, s3);
	return NULL;
}

ExprNode* ExprNode::GetExprNode_I(int type, int t)
{
	ExprNode* s = new ExprNode;
	s->unit(type, NULL, t, NULL, NULL);
	return s;
}

SelectWordList* SelectWordList::GetSelectWordList_D(SelectWordList* listp, char* s)
{
	SelectWordList* cur = listp;
	while (cur->Next())	cur = cur->Next();
	SelectWordList* node = GetSelectWordList(s);
	cur->SetNext(node);
	return listp;
}


