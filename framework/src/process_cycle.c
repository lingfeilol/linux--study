#define _GNU_SOURCE 
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<config.h> 
#include<settitle_proc.h>

//设置cpu亲和的头文件
#include<sched.h>


//主进程管理： 创建子进程，再死循环
void master_process_cycle()
{
    //创建子进程时，要避免信号的异步操作的干扰
    sigset_t set;
    sigemptyset(&set);

    sigaddset(&set,SIGHUP);
    sigaddset(&set,SIGINT);
    sigaddset(&set,SIGQUIT);
    sigaddset(&set,SIGABRT);
    sigaddset(&set,SIGBUS);
    sigaddset(&set,SIGUSR1);
    sigaddset(&set,SIGUSR2);
    sigaddset(&set,SIGSEGV);
    sigaddset(&set,SIGPIPE);
    sigaddset(&set,SIGALRM);
    sigaddset(&set,SIGTERM);
    sigaddset(&set,SIGCHLD);
    sigaddset(&set,SIGTTIN);
    sigaddset(&set,SIGTTOU);
    sigaddset(&set,SIGURG);
    sigaddset(&set,SIGWINCH);
    sigaddset(&set,SIGIO);
    sigprocmask(SIG_BLOCK,&set,NULL); //将信号位图里的信号阻塞掉,避免创建子进程的时候被打断

    //根据配置文件中的进程数，来确定可以创建几个子进程
    int worker_process_num=get_int_default("WorkerProc",4);
    //printf("woeker num:%d\n",worker_process_num);
    start_create_worker(worker_process_num,-1);//-1参数表示新创建子进程
    
    //设置主进程的名字
    set_proctitle("framework: maseter");
    sigemptyset(&set);
    // 父进程再此做循环动作
    for( ; ;)
    {
        sigsuspend(&set);//阻塞在这里，直到有信号到来，才被唤醒
        //因为阻塞信号一开始到来被阻塞后在被取消阻塞的时间中，是有延迟的，如果进程有其他操作，例如pause，就相当于这个信号消失了，信号就不会被处理，所以需要这个原子操作的函数，没有信号时就休眠，有信号来就被唤醒处理
        //此函数执行期间使用的新的自带set位图（自己定义的新的阻塞位图），执行之后恢复之
        //前设置的set阻塞位图
        //sigsuspend =pause()+自己的信号屏蔽 ，主要用于信号屏蔽切换的：
    }
}

//创建子进程，nums:子进程个数 type：-1表示创建子进程
void start_create_worker(int nums,int type)
{
    int i;
    for(i=0;i < nums ;i++)
    {
        create_worker(i,"framework: worker process",type);
    }
}
//真正创建子进程的逻辑
void create_worker(int inum,const char* procname,int type)
{
    pid_t pid = fork();
    switch (pid)
    {
        case -1:
          perror("fork"),exit(0);
        case 0://子进程逻辑
          worker_process_cycle(inum,procname); 
          break;
        default://父进程
          break;//回到上层调用继续创建子进程
    }
}


//切换父进程的信号屏蔽 ，设置cpu亲和（绑定到cpu处理器上）
static void worker_init(int inum)
{
    //子进程会继承父进程之前设置的的信号屏蔽，所以要先解除信号屏蔽，在可以正常接收信号
    sigset_t set;
    sigemptyset(&set);
    sigprocmask(SIG_SETMASK,&set,NULL);

    //接着设置cpu亲和 ，让每个子进程最大限度的利用核处理器的缓存
    cpu_set_t  cpuset;
    //操作cpuset的宏函数
    CPU_ZERO(&cpuset);
    int cpucount = get_int_default("Cpucount",1);
    CPU_SET(inum%cpucount ,&cpuset); // 每个进程合理的分配到每个处理器，来设置到cpuset
   //sched_setaffinity( __pid_t __pid, size_t __cpusetsize, const cpu_set_t *__cpuset  )
   //将进程绑定到设置好的响应的处理器上 ，pid为0表示设置当前进程
    sched_setaffinity(0,sizeof(cpuset),&cpuset);
}
// 子进程的执行逻辑
void worker_process_cycle(int inum,const char* procname)
{
    worker_init(inum); //子进程的初始化
    set_proctitle(procname); //设置子进程的标题
    for( ;  ; )
    {
        //这里将进行epoll_wait 事件循环监控 io等待过程
        sleep(1);
        printf("worker process %d\n",inum);
    }

}

