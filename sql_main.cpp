#include <iostream>
#include <stdio.h>
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <sstream>
#include <unordered_map>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <error.h>
#define LISTENQ 1024
#define RIOBUFSIZE 1024*8
#define MAXNLINE 1024*8
#define SBUFSIZE 16
#define NTHREADS 4
#include "sql_main.h"
// #ifndef TEST
// #define TEST
// #include "lex.yy.c"
// #endif
extern FILE* yyin;
extern int yyparse(void);
extern void scan_string(const char* str);
//extern YY_BUFFER_STATE yy_scan_string (yyconst char *yy_str  );
using namespace std;
vector<Where_Require> CnameNode;
bool Flag_Exec=true;
string ANS="";
void yyerror(const char* s, ...)
{
	ANS="";
	for(int i=0; s[i]!='\0'; i++)
		ANS+=s[i];
	ANS+='\n';
}


typedef struct
{
	int *buf;
	int n;
	int front,rear;
	sem_t mutex;
	sem_t items;
	sem_t slots;
}sbuf_t;
typedef struct
{
	int rio_fd,rio_cnt;
	char rio_buf[RIOBUFSIZE], *rio_bufptr;
}rio_t;


static sem_t mutex;
static int byte_cnt;
sbuf_t sbuf;

void P(sem_t *s)
{
	if(sem_wait(s) < 0)	fprintf(stderr, "sem_wait error: %s\n", strerror(errno)),exit(0);
} 
 void V(sem_t *s)
{
	if(sem_post(s) < 0)	fprintf(stderr, "sem_post error: %s\n", strerror(errno)),exit(0);
 } 

ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
	size_t nleft=n;
	size_t nwritten;
	char *buf = (char *)usrbuf;

	while(nleft > 0)
	{
		if((nwritten = write(fd, buf, nleft)) <= 0)
		{
			if(errno == EINTR)	nwritten=0;      //中断了 
			else return -1;
		}
		nleft -= nwritten;
		buf += nwritten;
	
	}
	return n;	
}
ssize_t rio_read(rio_t *rp, char *rc, size_t n)
{
	int cnt;
	while(rp->rio_cnt <= 0)
	{
		rp->rio_cnt=read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if(rp->rio_cnt < 0)
		{
			if(errno != EINTR)	return -1;
		}
		else if(rp->rio_cnt == 0)	return 0;
		else rp->rio_bufptr = rp->rio_buf;
	}
	cnt = n;
	if(rp->rio_cnt < n)	cnt=rp->rio_cnt;
	memcpy(rc, rp->rio_bufptr, cnt);
	rp->rio_bufptr+=cnt;
	rp->rio_cnt-=cnt;
	return cnt;
	
}

ssize_t rio_readlineb(rio_t *rio, void *usrbuf, size_t maxlen)
{
	char *buf=(char *)usrbuf, c;
	int n,rc;
	for(n=1; n<maxlen; n++)
	{
		if((rc = rio_read(rio, &c, 1)) == 1)
		{
			*buf++ = c;
			if(c=='\n')
			{
				n++;
				break;
			}
		}
		else if(rc == 0)
		{
			if(n==1)	return 0;
			else break;
		}
		else return -1;
	}
	*buf = 0;
	return n-1;
		
}

void rio_readinitb(rio_t *rio, int fd)
{
	rio->rio_fd=fd;
	rio->rio_cnt=0;
	rio->rio_bufptr=rio->rio_buf;
}

int open_listenfd(char *port)
{
    struct addrinfo hints,*p,*listp;
    int listenfd, optval=1,rc;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype=SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    //socket---->bind---->listen
    getaddrinfo(NULL, port, &hints, &listp);
   // if(listp == NULL) 
    for(p=listp; p; p=p->ai_next)
    {
        if((listenfd=socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) continue;
       if((rc= setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, & optval, sizeof(int))) < 0) printf("%s\n", strerror(errno));
        if(bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)     //printf("%s\n", strerror(errno)); //break;
        		break;
		close(listenfd);
    }
    freeaddrinfo(listp);
    if(!p)
    {
        return -1;
    }
    if(listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
        return -1;
    }
    return listenfd;
    
}


static void init_echo_cnt(void)
{
	sem_init(&mutex, 0, 1);
	byte_cnt=0;
}
void echo_cnt(int connfd)
{
	int n;
	char buf[MAXNLINE];
	rio_t rio;
	rio_readinitb(&rio, connfd);
	static pthread_once_t once = PTHREAD_ONCE_INIT;  //即使是在多线程程序中，此函数也只调用一次，由once参数控制  
	pthread_once(&once, init_echo_cnt);
	while((n=rio_readlineb(&rio, buf, MAXNLINE)) != 0)
	{
		string f="";
		P(&mutex);
		byte_cnt+=n;
		printf("server received %d (%d total) bytes on fd %d\n", n, byte_cnt, connfd);
		for(int p=0; p<n-1; p++)
		{
			f+=buf[p];
		}
		ANS="";
		 scan_string(f.c_str());
		cout<<f<<endl;
		 yyparse();
		//ANS+="\n---------------------------------------------------\n";
		V(&mutex);
		//rio_writen(connfd, buf, n);
		//ANS+="\n---------------------------------------------------\n";
		ANS+="$";
		rio_writen(connfd, (void *)ANS.c_str(), ANS.size());
	}
	
} 
void sbuf_init(sbuf_t *sp, int x)
{
	sp->buf=(int *)calloc(x, sizeof(int));
	sp->n=x;
	sp->front=sp->rear=0;
	sem_init(&sp->mutex, 0, 1);
	sem_init(&sp->slots, 0, x);
	sem_init(&sp->items, 0, 0);
}
void sbuf_insert(sbuf_t *sp, int item)
{
	P(&sp->slots);
	P(&sp->mutex);
	sp->buf[(++sp->rear) % (sp->n)]=item;
	V(&sp->items);
	V(&sp->mutex);
}
int sbuf_remove(sbuf_t *sp)
{
	int item;
	P(&sp->items);      //取出来以后可用项的数目减一 
	P(&sp->mutex);          //加互斥锁 
	item=sp->buf[(++sp->front)%(sp->n)];
	V(&sp->mutex);         //解开互斥锁 
	V(&sp->slots);        //取出来，空槽位加一	
	return item;
} 
void *thread(void *vargp)
{
	pthread_detach(pthread_self());
	while(1)
	{
		int connfd=sbuf_remove(&sbuf);   //取出连接符
		echo_cnt(connfd);
		
		close(connfd); 
	}
}



int main(int argc, char **argv)
{

	int i,listenfd,connfd;
	socklen_t clientlen;
	char client_hostname[MAXNLINE], client_port[MAXNLINE];
	struct sockaddr_storage clientaddr;
	pthread_t tid;
	if(argc != 2)
	{
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(0);
	}
	listenfd=open_listenfd(argv[1]);
	sbuf_init(&sbuf, SBUFSIZE);	
	//工作线程 
	for(i=0; i<NTHREADS; i++)
	{
		pthread_create(&tid, NULL, thread, NULL);  //创建对等线程 
	}
	while(1)
	{
		clientlen=sizeof(struct sockaddr_storage);
		connfd=accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen);
		sbuf_insert(&sbuf, connfd);	
		
	} 
}

