%option yywrap default yylineno case-insensitive



%{
#include "sql.tab.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "sql_main.h"

extern "C"
{
int yylex(void);
} 
%}






%x COMMENT

%%
<<EOF>> { yyterminate(); }
SELECT { return SELECT; }
FROM { return FROM; }
WHERE { return WHERE; }
CREATE { return CREATE; }
DATABASE { return DATABASE; }
USE { return USE; }
TABLE { return TABLE; }
DROP { return DROP; }
INSERT { return INSERT; }
INTO { return INTO; }
VALUES { return VALUES; }
AND { return AND; }
OR { return OR; }
DELETE { return DELETE; }
UPDATE { return UPDATE; }
SET { return SET; }
SCHEMA {return SCHEMA;}
INT { return INT; }
CHAR { return CHAR; }
ROLE { return ROLE; }
GRANT { return GRANT; }
TO { return TO; }
ON { return ON; }
PRIMARY {return PRIMARY;}
KEY {return KEY;}
SHOW {return SHOW;}
TABLES {return TABLES;}
NOT {return NOT;}
NULLX {return NULLX;}
REVOKE { return REVOKE; }
CASCADE { return CASCADE; }
WITH { return WITH; }
OPTION { return OPTION; }
EXIT { yyterminate(); }


-?[0-9]+    {yylval.intval = atoi(yytext); return INTNUM;}


-?[0-9]*\\.[0-9]* | 
-?[0-9]+E[-+]?[0-9]+ |
-?[0-9]+\\.[0-9]*E[-+]?[0-9]+	{yylval.floatval=atof(yytext); return APPROXNUM;}


TRUE    {yylval.intval=1; return BOOL;}
FALSE   { yylval.intval = 1; return BOOL; }




[A-Za-z][A-Za-z0-9_]* { yylval.strval = strdup(yytext); return NAME; }
'([^'\n]*)' {yylval.strval = strdup(yytext); yylval.strval=yylval.strval+1;
							yylval.strval[strlen(yylval.strval)-1]='\0'; return STRING; }   




[-+*/(),.;] {return yytext[0]; }
"="     { yylval.subtok = 1; return COMPARSION; }
"!="    { yylval.subtok = 2; return COMPARSION; }
"<"     { yylval.subtok = 3; return COMPARSION; }
">"     { yylval.subtok = 4; return COMPARSION; }
"<="    { yylval.subtok = 5; return COMPARSION; }
">="    { yylval.subtok = 6; return COMPARSION; }



"/*"    {BEGIN(COMMENT);}
<COMMENT>"*/"    ;
<COMMENT>"([^*]\n)+|."    ;
<COMMENT><<EOF>>	{yyerror("Unterminated comment\n");}


%%
void scan_string(const char* str)
{
    yy_switch_to_buffer(yy_scan_string(str));
}