#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"threadpool.h"

void* fun(void* args)
{
    char* msg=args;
    printf("thread %p,%s\n",pthread_self(),msg);
    sleep(5);
}

int main()
{
    threadpool_t pool;
    threadpool_init(&pool,4);
    threadpool_add_task(&pool,fun,"hello");
    threadpool_destory(&pool);
    return 0;
}
