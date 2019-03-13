#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<unistd.h>
#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<pthread.h>

#define MAX_EVENT_NUMBER 1024
#define BUFFER_SIZE 10
/* 将文件描述符设置成非阻塞的 */
int setnonblocking(int fd)
{
	int old_option=fcntl(fd,F_GETFL);
	int new_option=old_option|O_NONBLOCK;
	fcntl(fd,F_SETFL,new_option);
	return old_option;
}
/* 将文件描述符fd上的EPOLLIN注册到epollfd指示的epoll内核时间表中，参数enable_et指定是否对fd启用ET模式 */
void addfd(int epollfd,int fd,bool enable_et)
{
	epoll_event event;
	event.data.fd=fd;
	event.events=EPOLLIN;
	if(enable_et)
	{
		event.events|=EPOLLET;
	}
	epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
	setnonblocking(fd);
}
/* LT模式的工作流程 */
void lt(epoll_event* events,int number,int epollfd,int listenfd)
{
	char buf[BUFFER_SIZE];
	for(int i=0;i<number;i++)
	{
		int sockfd=event[i].data.fd;
		if(sockfd==listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength=sizeof(client_address);
			int connfd=accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
			addfd(epollfd,connfd,false);    /* 对connfd禁用ET模式 */
		}
		else if(events[i].events&EPOLLIN)
		{
			/* 只要socket读缓存中还有未读出的数据，这段代码就被触发 */
			printf("event trigger once\n");
			memset(buf,'\0',BUFFER_SIZE);
			int ret=recv(sockfd,buf,BUFFER_SIZE-1,0);
			if(ret<=0)
			{
				close(fd);
				continue;
			}
			printf("get %d bytes of content: %s\n",ret,buf);
		}
		else
		{
			printf("something else happened\n");
		}
	}
}
/* ET模式的工作流程 */
void et(epoll_event* events,int number,int epollfd,int listenfd)
{
	char buf[BUFFER_SIZE];
	for(int i=0;i<number;i++)
	{
		int sockfd=events[i].data.fd;
		if(sockfd==listenfd)
		{
			struct sockaddr_in client_address;
			socklen_t client_addrlength=sizeof(client_address);
			int connfd=accept(listenfd,(struct sockaddr*)&client_address,&client_addrlength);
			addfd(epollfd,connfd,true);
		}
	}
}