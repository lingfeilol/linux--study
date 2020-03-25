#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<stdlib.h>
void sigcallback(int signo)
{
  printf("signo is %d \n",signo);
  wait(NULL);
}
int main()
{
  signal(SIGCHLD,sigcallback);
  pid_t pid=fork();
  if(pid<0)
  {
    perror("fork");
    return -1;
  }
  else if(pid==0)
  {
    //child
    sleep(4);
    printf("i am child \n");
    _exit(0);//进程终止，一旦子进程退出，便会产生sigchld信号，此时便会另一个执行流去调用自定义信号处理函数里的wait函数，来避免僵尸状态
    
  }
  //father
  while(1)
  {
    sleep(4);
    printf("i am father\n");
  }
  
  return 0;
}
