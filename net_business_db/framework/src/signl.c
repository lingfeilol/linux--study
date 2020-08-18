#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<signal.h>
//定义信号，将信号的值，名字，对应的shell信号命令，信号处理函数地址在一个结构体中
typedef struct{
    int signo;             //信号的值 SIGINT
    char* sigame;         //信号的名字 "SIGINT"
    char* command;      //触发命令
    void (*handler)(int signo, siginfo_t *info , void *ucontext);  //对应的信号处理函数
}signal_t;

void sig_headler(int signo,siginfo_t *info , void *ucontext)
{
    printf("recv %d\n",signo);
}

//把要处理的信号先保存起来,挨个注册
signal_t sig_arr[]={
    {SIGHUP,  "SIGHUP",NULL,sig_headler}, 
    {SIGINT,  "SIGINT",NULL,sig_headler}, 
    {SIGQUIT, "SIGQUIT",NULL,sig_headler}, 
    {SIGCHLD, "SIGCHLD",NULL,sig_headler}, 
    {SIGTERM, "SIGTERM",NULL,sig_headler}, 
    // .....
    {SIGSYS,"SIGSYS",NULL,NULL},  //不需要信号处理的信号，回调函数设置为NULL
    {0, NULL ,NULL,NULL} 
};

//信号的初始化(注册) 
int init_signal()
{
    signal_t* sig=NULL;
    struct sigaction act;//填充回调函数的结构体
    for(sig=sig_arr; sig->signo!=0;sig++)
    {
        memset(&act,0x00,sizeof(struct sigaction));
        if(sig->handler== NULL)
        {
            act.sa_sigaction =(void (*)(int ,siginfo_t*,void*)) (SIG_IGN);  //信号忽略
        }
        else 
        {
            act.sa_sigaction = sig_headler;
        }
        act.sa_flags = 0;
        sigemptyset(&act.sa_mask); //将阻塞位图置空
        sigaction(sig->signo,&act,NULL); 
    }
}










