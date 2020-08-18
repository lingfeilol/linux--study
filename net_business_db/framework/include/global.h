#ifndef __GLOBAL_H__
#define __GLOBAL_H__ 
//存放转移环境变量时用的全局变量

extern char **g_os_argv; //定义指向命令行参数的起始位置

extern int g_envmemlen;  //环境变量的空间大小
extern char* g_envptr;

#endif //__GLOBAL_H__
