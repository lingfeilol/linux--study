#include"threadpool.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#if 0 
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
#endif

//线程回调函数
void *routine(void *arg)
{
    threadpool_t* pool=(threadpool_t*)arg;

    while(1) //循环判断是否有资源可用
    {
        pthread_mutex_lock(&(pool->mutex));
        pool->idle++;//刚上来是空闲的

        while(pool->head ==NULL && pool->quit ==0 ) //需要循环判断资源是否可用 或者是否退出
        {
            pthread_cond_wait(&(pool->cond),&(pool->mutex));
        }
        //被唤醒后，意味着线程开始工作
        pool->idle--;
        if(pool->head != NULL) //线程有任务可做
        {
            node_t* tmp=pool->head;
            pool->head=tmp->next;
            //拿到任务后，不再访问临界区资源，且工作时间可能比较长，所以解锁，让线程并发去工作
            pthread_mutex_unlock(&(pool->mutex));
            tmp->callback(tmp->args);//执行用户规定的任务
            free(tmp);
            pthread_mutex_lock(&(pool->mutex));
        }
        
        //无任务且 判断是否退出
        if(pool->quit ==1 && pool->head == NULL)
        {
            pool->counter--;
            if( pool->counter == 0)//最后一个线程离开的时候
            {
                pthread_cond_signal(&(pool->cond));
            }
            pthread_mutex_unlock(&(pool->mutex));
            break;//线程跳出循环 ，即退出
        }
        pthread_mutex_unlock(&(pool->mutex));
    }
}



//初始化线程池
void threadpool_init(threadpool_t *pool,int max_thread)
{
    pthread_cond_init(&(pool->cond),NULL);
    pthread_mutex_init(&(pool->mutex),NULL);
    pool->head=NULL;
    pool->tail=NULL;
    pool->max_thread=max_thread;
    pool->counter=0;
    pool->idle=0;
    pool->quit=0;
}

//往任务队列里添加任务
//如果有空闲线程，就直接回调这个函数
//没有，且当前线程数没有达到规定最大值，就创建新线程来执行任务
void threadpool_add_task(threadpool_t* pool,void *(*task)(void*),void* args)
{
    node_t* newnode=(node_t*)malloc(sizeof(node_t));
    memset(newnode,0x00,sizeof(node_t));
    newnode->args=args;
    newnode->callback=task;
    
    //多个线程对应的这个任务队列，属于临界区资源，注意线程安全问题
    pthread_mutex_lock(&(pool->mutex));
    if(pool->head==NULL)
    {
        pool->head=newnode;
    }
    else 
    {
       pool->tail->next=newnode;
    }
    pool->tail=newnode;
    
    //判断是否有线程可以执行任务
    if(pool->idle > 0)
    {
        //要是有空闲线程，那就说明有线程阻塞到pthread_cond_wait() (生产者消费者模型)
        //那就去唤醒他，然后执行当前任务
        pthread_cond_signal(&(pool->cond));
    }
    else if( pool->counter < pool->max_thread )
    {
        //要是没有空闲线程，且线程数是小于最大线程个数时，便可以另开一个线程来执行此任务
        pthread_t tid;
        pthread_create(&tid,NULL,routine,(void*)pool);
        pthread_detach(tid);
        pool->counter++;
    }
    else 
    {
        //达到上限，只能等待
    }
    pthread_mutex_unlock(&(pool->mutex));

}
//销毁线程池
void threadpool_destory(threadpool_t* pool)
{
    if( pool->quit ==1  )
        return ;
    pthread_mutex_lock(&(pool->mutex));
    pool->quit=1;
    if(pool->counter > 0)
    {
        if(pool->idle >0) //当空闲线程大于0，才去通知
        {
            pthread_cond_broadcast(&(pool->cond));//给所有在等待队列的线程通知唤醒
        }
        //阻塞在这里，直到所有线程池中的线程销毁，才离开
        while(pool->counter > 0)
        {
            pthread_cond_wait(&(pool->cond),&(pool->mutex));
        }
    }
    pthread_mutex_unlock(&(pool->mutex));
    pthread_cond_destroy(&(pool->cond));
    pthread_mutex_destroy(&(pool->mutex));
}

