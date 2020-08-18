#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<global.h>
#include<settitle_proc.h>
extern char** environ;


//给环境变量搬家  (以访给的命令行参数太长而把环境变量的空间给覆盖了)
void change_env()
{
    //知道指向环境变量的指针
    //求得环境变量的大小，再在堆上开一个一样大小的空间，再将拷贝数据，最后再改变指针的指向
    int i;
    for(i=0; environ[i]!=NULL;i++)
    {
        g_envmemlen+=strlen(environ[i])+1;        
    }
    //开空间
    g_envptr=malloc(sizeof(char)*g_envmemlen);
    memset(g_envptr,0x00,g_envmemlen);
    //拷贝
    char* tmp=g_envptr;
    for(i=0;environ[i]!=NULL;i++)
    {
        int len=strlen(environ[i])+1;
        strcpy(tmp,environ[i]);
        //指向新的位置
        environ[i]=tmp;
        tmp+=len;
    }
}
//设置进程的名字的函数
void set_proctitle(const char* name)
{
    //先考虑标题长度，万一比环境变量加命令行参数的空间还大，就报错，太长了
    int title_len = strlen(name);

    //计算命令行参数的长度
    int arg_len=0;
    int i;
    for(i=0;g_os_argv[i]!=NULL;i++)
    {
        arg_len+=strlen(g_os_argv[i])+1;
    }
    //比较
    if(arg_len + g_envmemlen < title_len )
        return ; //太长了
    //名字只需要一行，再把后面的全部置空
    g_os_argv[1]=NULL;
    char* tmp=g_os_argv[0];
    strcpy(tmp,name);//拷贝新标题
    //剩下的空间全部置空
    tmp+=title_len;
    memset(tmp,0x00,arg_len + g_envmemlen - title_len);
      
}
