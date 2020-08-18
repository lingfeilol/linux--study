#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<fcntl.h>
    //编写守护进程的一般步骤步骤：
    //（1）在父进程中执行fork并exit推出；
    //（2）在子进程中调用setsid函数创建新的会话；
    //（3）在子进程中调用chdir函数，让根目录 ”/” 成为子进程的工作目；防止不在根目录上，被删盘时删除掉
    //（4）在子进程中调用umask函数，设置进程的umask为0；
    //（5）在子进程中关闭任何不需要的文件描述符 指向黑洞文件 
int my_daemon()
{
    switch (fork())
    {
        case 0: 
          break;  //子进程设置为守护进程
        default:
          exit(0); //父进程退出
    }
    if(setsid() == -1)
        perror("setsid"),exit(0);
    
    chdir("/");
    umask(0);//重新设置子进程的创建文件时的权限掩码
    
    int fd=open("/dev/null",O_RDWR);
    dup2(fd,0); // 或者 dup2(fd,STDIN_FILENO)
    dup2(fd,1);
    return 0;
}
