#include"func.h"
int make_child(pData_t p,int process_num)
{
	int i;
	pid_t pid;
	int fds[2];
	for(i=0;i<process_num;i++)
	{
		socketpair(AF_LOCAL,SOCK_STREAM,0,fds);
		pid=fork();
		if(0==pid)
		{
			close(fds[1]);//关闭写
			child_handle(fds[0]);
		}
		close(fds[0]);
		p[i].pid=pid;
		p[i].fds=fds[1];
		p[i].busy_flag=0;
	}
	return 0;
}
void child_handle(int fds)
{
	int new_fd;
	int exit_flag=0;
	char notbusy_flag=1;
	while(1)
	{
		recv_fd(fds,&new_fd,&exit_flag);
		if(-1==exit_flag)
		{
			exit(0);
		}
		tran_file(new_fd);
		write(fds,&notbusy_flag,1);
	}
}
