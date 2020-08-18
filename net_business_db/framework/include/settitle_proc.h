#ifndef __SETTITLE_PROC_H_
#define __SETTITLE_PROC_H_

//给环境变量搬家  (以访给的命令行参数太长而把环境变量的空间给覆盖了)
void change_env();
//设置进程的名字的函数
void set_proctitle(const char* name);
#endif //__SETTITLE_PROC_H_
