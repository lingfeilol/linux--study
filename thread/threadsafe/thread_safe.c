#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define THREADCOUNT 4  //模拟四个线程执行流

int g_tickes = 100;//访问全局变量临界资源
pthread_mutex_t lock;   //定义互斥量

void* ThreadStart(void* arg)
{
    (void)arg;
    while(1)
    {
        // 1 加锁
        pthread_mutex_lock(&lock);
        if(g_tickes > 0)
        {
            g_tickes--; 
            usleep(100000);//模拟等待
            printf("i am thread [%p], i have ticket num is [%d]\n", pthread_self(), g_tickes + 1);
        }
        else
        {
            //假设有一个执行流判断了g_tickets之后发现，g_tickets的值是小于等于0的
            //则会执行else逻辑，直接就被break跳出while循环
            //跳出while循环的执行流还加着互斥锁
            //所以在所有有可能退出线程的地方都需要进行解锁操作
            pthread_mutex_unlock(&lock);
            break;
        }
        pthread_mutex_unlock(&lock); //用完后解锁
    }
    return NULL;
}

int main()
{
    pthread_mutex_init(&lock, NULL);//初始化互斥锁
    pthread_t tid[THREADCOUNT]; //声明线程
    int i = 0;
    for(; i < THREADCOUNT; i++)
    {
        int ret = pthread_create(&tid[i], NULL, ThreadStart, NULL);//创建线程
        if(ret < 0)
        {
            perror("pthread_create");
            return 0;
        }
    }
    for(i = 0; i < THREADCOUNT; i++)
    {
        pthread_join(tid[i], NULL); //等待线程退出回收资源
    }

    pthread_mutex_destroy(&lock);//销毁互斥锁
    return 0;
}
