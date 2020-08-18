#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<config.h>

extern char** environ; //声明指向环境的指针变量，系统规定的
char** g_os_argv; //定义指向命令行参数的起始位置
int g_envmemlen;  //环境变量的空间大小
char* g_envptr;   //指向堆上新开空间的指针

int main(int argc, char *argv[])
{
    g_os_argv = argv;
    load("my.conf");//加载配置文件，即将磁盘上的配置文件加载到内存里（这里用链表保存）

    change_env();//当前进程环境变量搬家
   // set_proctitle("master");
    init_signal();
    
    //进入主循环之前，判断是否要设置为守护进程运行
    if(get_int_default("Daemon",0)== 1)
        my_daemon(); 
    master_process_cycle();//进入主循环，创建子进程
    while(1)
    {
        pause();
    }
    return 0;
}
