#welcome to sql makefile
LEX = flex
YACC = bison
CC = g++

sql: lex.yy.o sql.tab.o sql_select.o sql_update_delete.o sql_insert.o sql_create.o sql_main.o
	$(CC) lex.yy.o sql.tab.o Do_select.o Do_update_delete.o Do_create.o Do_insert.o sql_main.o -o sql -std=c++11 -pthread -lfl

sql_main.o: sql_main.cpp sql_main.h
	$(CC) -c sql_main.cpp -std=c++11

sql_update_delete.o: Do_update_delete.cpp sql_main.h Do_select.cpp
	$(CC) -c Do_update_delete.cpp -std=c++11

sql_insert.o: Do_insert.cpp sql_main.h
	$(CC) -c Do_insert.cpp -std=c++11
	
sql_create.o: Do_create.cpp sql_main.h
	$(CC) -c Do_create.cpp -std=c++11
    
sql_select.o: Do_select.cpp  sql_main.h                                                                                        
	$(CC) -c Do_select.cpp -std=c++11

lex.yy.o: lex.yy.c sql.tab.h
	$(CC) -c lex.yy.c -std=c++11

sql.tab.o: sql.tab.c
	$(CC) -c sql.tab.c -std=c++11 

sql.tab.c sql.tab.h : sql.y
	$(YACC) -d sql.y

lex.yy.c: sql.l
	$(LEX) sql.l 

clean:
	rm -f *.o lex.* sql.tab.* *.o
	
