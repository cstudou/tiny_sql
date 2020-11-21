#include "sql_main.h"

using namespace std;
CreateTable* Make_Create_Table(char* s, TableMessage* f)
{
	CreateTable* sf = new CreateTable;
	sf->unit(s, f);
	return sf;
}
TableMessage* Make_Create_Definition(char* s1, int& s2, int& s3, int &s4)
{
	TableMessage* sf = new TableMessage;
	sf->unit(s1, s2, s3, s4);
	return sf;
}


void SaveName(char* s)
{
	Where_Require u;
	u.Column = s;
	CnameNode.push_back(u);
}

void Do_Create(CreateTable*& s)
{
	/* 遍历所有表项 */
	struct stat buf1, buf2, buf3;
	int res = 0;
	string op = "base.info";
	string d = s->GetName();
	string v = d + ".config";   //配置信息
	
	ofstream InBaseconfig(op.c_str(), ios::app);         //打开配置文件
	InBaseconfig<<d<<" ";
	d = d + ".dat";   //数据信息
	
	
	if(stat(d.c_str() ,&buf1) == 0)
    {
        yyerror("该文件表已存在！");
        return ;
    }
	
    if(stat(v.c_str(), &buf2) == 0)
    {
        yyerror("该文件表已存在！");
        return ;
    }
	ofstream Indata(d.c_str(), ios::ate | ios::binary);         //打开文件
	ofstream Inconfig(v.c_str(), ios::app);         //打开配置文件
    
	auto cur = s->GetNext(); //读取出表的元素

	while (cur)
	{
		Indata.write(cur->GetName(), cur->Get_Message_Length());    //写入文件
		/************************************************************************/
			/*
			 Indata.write("siz", 8);
			 Indata.write("sundy", 8);
			 Indata.write("hehe", 8);
			*/		 
		/************************************************************************/
		Inconfig << cur->Get_Message_Length() << " " << cur->Get_Message_Type() << " " <<cur->Get_Message_bind()<< " ";


		cur = cur->GetNext();
	}
	char vv[10] = { '0' };  // 记录条数
	Indata.write(vv, 8);  //最后8字节保存记录条数
	Indata.close();
	Inconfig.close();
}
void DO_SHOW()
{
	ANS="";
	string op = "base.info", res;
	ifstream Inconfig(op.c_str());         //打开配置文件
	if (Inconfig.good() == 0)
	{
		yyerror("表信息查询失败！");
		return;
	}
	ANS+="\n------------------\n";
	while(Inconfig>>res)
	{
		ANS+=res;
		ANS+='\n';
	}
	ANS+="\n------------------\n";
}