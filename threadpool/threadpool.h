#ifndef __THREADPOOL_H_
#define __THREADPOOL_H_

#include<pthread.h>
//任务节点 (链表队列)
typedef struct node {
    void *(*callback)(void* args);
    void *args;
    struct node* next;
}node_t;
//线程池结构体
typedef struct 
{
    pthread_cond_t cond;    //条件变量
    pthread_mutex_t mutex;  //互斥锁
    node_t* head;           //任务队列的对头
    node_t* tail;           //队尾
    int max_thread;         //线程池最多允许创建的线程个数
    int counter;            //线程池当前的线程个数
    int idle;               //空闲线程的个数
    int quit;               //为1 则销毁线程池
}threadpool_t;
//初始化线程池
void threadpool_init(threadpool_t *pool,int max_thread);

//往任务队列里添加任务
void threadpool_add_task(threadpool_t* pool,void *(task)(void*),void* args);
//销毁线程池
void threadpool_destory(threadpool_t* pool);
#endif //__THREADPOOL_H_
