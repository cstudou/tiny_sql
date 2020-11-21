/*表达式*/
%{
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "sql_main.h"


#include <vector>
using namespace std;



extern "C"
{

extern int yylex(void);
}

%}

%union
{
	int subtok;
	char *strval;
	int intval;
	double floatval;
	class ExprNode* EXPR;
	class SelectWordList* SELECTWORDLIST;
	class SelectNode* SELECTNODE;
	class InsertNode* insertnode;
	class TableMessage* tableMessage;
	class CreateTable* createTable;
	class UpdateNode* UPDATENODE;
}

%token <strval> NAME
%token <strval> STRING
%token <intval> INTNUM
%token <intval> BOOL
%token <floatval> APPROXNUM

%left OR
%left AND
%left <subtok> COMPARSION /* = != < > <= >= */
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS


%token CREATE
%token DATABASE
%token DATABASES
%token SHOW
%token USE
%token TABLE
%token TABLES
%token DROP
%token INSERT
%token INTO
%token VALUES
%token SELECT
%token FROM
%token WHERE
%token ADD
%token OR
%token DELETE
%token UPDATE
%token SET
%token IF_EXISTS
%token SCHEMA
%token COMPARSION
%token INT
%token CHAR
%token ROLE
%token GRANT
%token TO
%token ON
%token REVOKE
%token CASCADE
%token WITH
%token OPTION 
%token KEY
%token PRIMARY
%token NOT
%token NULLX 


%type <insertnode> insert_stmt
%type <SELECTWORDLIST> select_expr_list table_references opt_insert_columns column_list
%type <EXPR> expr opt_where insert_vals insert_vals_list update_asgn_list
%type <SELECTNODE> select_stmt
%type <UPDATENODE> update_stmt delete_stmt
%type <strval> table_reference table_factor
%type <createTable> create_table_stmt
%type <tableMessage> create_col_list create_definition
%type <intval> data_type opt_length opt_if_not_exist column_atts
%start stmt_list

%%
expr: NAME {SaveName($1); $$ = Make_expr_node_C(0, $1);}
	| STRING {$$ = Make_expr_node_C(1, $1);}
	| INTNUM {$$ = Make_expr_node_I(2, $1);}
	;
	
expr: '(' expr ')'	{$$ = $2;}
	|  expr '+' expr	{$$ = Make_expr_node_N($1, $3, 1);}
	|  expr '-' expr	{$$ = Make_expr_node_N($1, $3, 2);}
	|  expr '*' expr	{$$ = Make_expr_node_N($1, $3, 3);}
	|  expr '/' expr	{$$ = Make_expr_node_N($1, $3, 4);}
	|  expr COMPARSION expr	{$$ = Make_expr_node_LR($1, $3, $2+2);}
	|  expr AND expr	{$$ = Make_expr_node_LR($1, $3, 9); }
	|  expr OR expr		{$$ = Make_expr_node_LR($1, $3, 10);}
	;


/*start*/
stmt_list:
	| stmt_list ';' { printf("> "); }
	| stmt_list stmt ';' { printf("> "); }
	| stmt_list error { yyclearin; yyerrok; printf("> "); }
	;
	
/*select 相关规则*/
stmt: select_stmt {}
	;
stmt: SHOW TABLES { DO_SHOW();}
	;
select_stmt: SELECT select_expr_list FROM table_references opt_where { $$ = Make_select_tree($2,$5,$4);}
	;
		
		
select_expr_list: NAME	{$$ = Make_word_expr_list($1);}
	| select_expr_list ',' NAME	{$$ = Make_word_list_D($1, $3);}   /**递归创建*/
	| '*'	{$$ = NULL;}
	;
	
table_references: table_reference	{	$$ = Make_word_expr_list($1);}
	| table_references ',' table_reference {$$ = Make_word_list_D($1, $3);}
	;	
	
table_reference: NAME {$$ = $1;}
	;
	
opt_where:	{$$ = NULL;}
	| WHERE expr {$$ = $2;}
	;
	

/*       create table相关规则    */
stmt: create_table_stmt {Do_Create($1);}
	;

create_table_stmt: CREATE TABLE opt_if_not_exist table_factor '(' create_col_list ')'
		{	
			$$ = Make_Create_Table($4, $6);
		}
		;

create_col_list: create_definition{}
	| create_col_list ',' create_definition 
	 {
		TableMessage *p;
		 $$ = p->Merge($1, $3);
	 }
	 ;
create_definition: NAME data_type opt_length column_atts
	{
		$$ = Make_Create_Definition($1,$2,$3,$4);
	}
	;

table_factor: NAME{$$ = $1;}
	;
data_type: INT {$$ = 1;}
	| CHAR {$$ = 0;}
	;
opt_length: {$$ = 8;}
	| '(' INTNUM ')' {$$ = $2;}
	;
opt_if_not_exist:{$$ = 0;}
	| IF_EXISTS { $$ = 1; }
	;
/*   1:默认 2:not null 3:pk*/
column_atts: { $$ = 1; }
	| NOT NULLX 	{ $$ = 2;}
	| PRIMARY KEY 	{ $$ = 3; }
	;

/* insert 相关规则*/
stmt: insert_stmt {InsertExec($1);}
	;

insert_stmt: INSERT INTO table_reference opt_insert_columns VALUES insert_vals_list
	{
		$$ = Make_insert_tree($3, $4, $6);
	}
	;

opt_insert_columns:
	'(' column_list ')'
	{
		$$ = $2;
	}
	|	{$$ = NULL;}
	;
	
column_list: NAME { $$ = Make_word_expr_list($1);}
	| column_list ',' NAME
	{
		$$ = Make_word_list_D($1, $3);
	}
	;

insert_vals_list: '(' insert_vals ')' { $$ = $2;  }
	| insert_vals_list ',' '(' insert_vals ')'
	{
		$$ = Merge_insert_vals($1, $4);
	}
	;
	
insert_vals: expr {$$ = $1;}
	| insert_vals ',' expr 
	{
		$$ = Make_insert_vals($1, $3);	
	}
	;

/*  update相关规则  */
stmt: update_stmt {Do_update($1);}
	;

update_stmt: UPDATE table_references 
		     SET update_asgn_list
		     opt_where {$$ = Make_update_node($2, $4, $5);}
	;

update_asgn_list: NAME COMPARSION expr {$$ = Make_update_list($1,$3);}
	| update_asgn_list ',' NAME COMPARSION expr {$$ = Merge_update_list($1, $3, $5);}

	;



/*stmt:delete statement*/

stmt: delete_stmt {Do_delete($1);}
	;

delete_stmt: DELETE FROM table_references opt_where
		{
			$$ = Make_update_node($3, NULL, $4);
		}
	;


%%
