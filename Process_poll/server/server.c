#include"func.h"//客户端升级情况下，kill子进程
int exit_fds[2];
void sigfunc(int signum)
{
	char exit_flag='c';
	write(exit_fds[1],&exit_flag,1);
}
int main(int argc,char **argv)
{
	pipe(exit_fds);
	args_check(argc,4);
	int  process_num=atoi(argv[3]);
	pData_t p=(pData_t)calloc(process_num,sizeof(Data_t));
	make_child(p,process_num);//创建子进程
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	int ret;
	int reuse=1;
	ret=setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(int));
	check_error(-1,ret,"setsockopt");
	int epfd=epoll_create(1);
	struct epoll_event event,*evs;
	evs=(struct epoll_event*)calloc(process_num+2,sizeof(struct epoll_event));
	event.events=EPOLLIN;
	event.data.fd=sfd;
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);
	check_error(-1,ret,"epoll_ctl");
	event.events=EPOLLIN;
	event.data.fd=exit_fds[0];
	ret=epoll_ctl(epfd,EPOLL_CTL_ADD,exit_fds[0],&event);
	check_error(-1,ret,"epoll_ctl");
	int i;//把子进程加入epoll的兴趣队列中
	for(i=0;i<process_num;i++)
	{
		event.data.fd=p[i].fds;
		ret=epoll_ctl(epfd,EPOLL_CTL_ADD,p[i].fds,&event);
		check_error(-1,ret,"epoll_ctl");
	}
	struct sockaddr_in ser;
	bzero(&ser,sizeof(ser));
	ser.sin_family=AF_INET;
	ser.sin_port=htons(atoi(argv[2]));
	ser.sin_addr.s_addr=inet_addr(argv[1]);
	ret=bind(sfd,(struct sockaddr*)&ser,sizeof(ser));
	check_error(-1,sfd,"bind");
	listen(sfd,process_num);//可以是任何值别大于子进程数就行
	int read_fd_num;
	int new_fd;
	char notbusy_flag;
	int j;
	signal(SIGUSR1,sigfunc);//注册捕捉信号函数
	while(1)
	{
		read_fd_num=epoll_wait(epfd,evs,process_num+1,-1);
		for(i=0;i<read_fd_num;i++)
		{
			if(evs[i].data.fd==sfd)
			{
				new_fd=accept(sfd,NULL,NULL);
				for(j=0;j<process_num;j++)
				{
					if(0==p[j].busy_flag)
					{
				        send_fd(p[j].fds,new_fd,0);
						p[j].busy_flag=1;
						printf("pid=%dis busy\n",p[j].pid);
						break;
					}
				}
				close(new_fd);
			}
			if(evs[i].data.fd==exit_fds[0])
			{
				close(sfd);
				for(j=0;j<process_num;j++)
				{
					send_fd(p[j].fds,0,-1);
				}
				for(j=0;j<process_num;j++)
				{
					wait(NULL);
				}
				printf("exit success\n");
				return 0;
			}
			for(j=0;j<process_num;j++)
			{
				if(evs[i].data.fd==p[j].fds)
				{
					read(p[j].fds,&notbusy_flag,1);
					p[j].busy_flag=0;
					printf("pid=%d is not busy\n",p[j].pid);
				}
			}
		}
	}
}
