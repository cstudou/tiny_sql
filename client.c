#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#define LISTENQ 1024
#define RIOBUFSIZE 1024*8
#define MAXNLINE 1024*8

typedef struct
{
	int rio_fd,rio_cnt;
	char rio_buf[RIOBUFSIZE], *rio_bufptr;
}rio_t;



ssize_t rio_writen(int fd, void *usrbuf, size_t n)
{
	size_t nleft=n;
	size_t nwritten;
	char *buf = usrbuf;
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
	char *buf=usrbuf, c;
	int n,rc;
	for(n=1; n<maxlen; n++)
	{
		if((rc = rio_read(rio, &c, 1)) == 1)
		{
			*buf++ = c;
			if(c=='$')
			{
                *(--buf)='\0';
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
int open_clientfd(char *hostname, char *port)
{
	int clientfd;
	struct addrinfo *p,*listp,hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG | AI_PASSIVE;
	//ai_addrconfig 只有当主机配置为ipv4时，getaddrinfo返回ipv4
	//ai_passive 使得getaddrinfo默认返回套接字地址
	//ai_numericserv 强制第二个参数service默认为端口号 
	
	
	getaddrinfo(hostname, port, &hints, &listp);
	for(p=listp; p; p=p->ai_next)
	{
		if((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)	continue;
		// 通过connect建立与服务器的连接
		// int connect(int clientfd, const struct sockaddr *addr, socklen_t adddrlen)
		if(connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)	break;
		close(clientfd); 
	}
	freeaddrinfo(listp);
	if(!p)	return -1;
	else return clientfd;      //客户端不需要监听 
}

int main(int argc, char **argv)
{
	int m,clientfd;
	char *host, *port, buf[MAXNLINE];
	rio_t rio;
	
	if(argc != 3)
	{
		fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
		return 0;		
	}	
	host = argv[1], port = argv[2];
	clientfd = open_clientfd(host, port);	
	rio_readinitb(&rio, clientfd);
	while(fgets(buf, MAXNLINE, stdin) != NULL)
	{
        //printf("%s--\n", buf);
		rio_writen(clientfd, buf, strlen(buf));
	  // if((m=rio_readlineb(&rio, buf, MAXNLINE))!=0 )  //服务器写回来再让客户端读 
      // {
           rio_readlineb(&rio, buf, MAXNLINE); 
            fputs(buf, stdout);
	       // memset(buf, '\0', sizeof(buf));   
           // printf("%d---\n", m);
       
    }
	close(clientfd);
	return 0;
} 
