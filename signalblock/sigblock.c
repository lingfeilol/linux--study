#include<stdio.h>
#include<signal.h>
#include<unistd.h>
void sigcallback(int signo)
{
  printf("this signo is %d\n",signo);
}
int main()
{
  signal(2,sigcallback);  //测试非可靠信号阻塞时怎么处理 
  signal(37,sigcallback);//测试可靠信号
  sigset_t set,oldset;
  sigemptyset(&set);//位图初始化 全为0
  sigemptyset(&oldset);//之前的旧位图也初始化为0
  sigfillset(&set);//初始化为全1


  sigprocmask(SIG_BLOCK,&set,&oldset);//设置为阻塞态， 由于set全为1 oldset全为0 或运算后新的位图还是全为1 ，即所有信号都是阻塞态
  getchar(); 
  sigprocmask(SIG_UNBLOCK,&set,NULL);   //设置非阻塞态，与操作（~set）必为0，及此时所有信号被跟改为非阻塞态
  while(1)
  {
    sleep(1);
  }
  return 0;
}
