#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#define THREADCOUNT 2   //每类线程个数
//0代表没有资源可用
//1代表还有资源可用
int g_val = 0;

pthread_mutex_t lock;  //定义互斥锁
pthread_cond_t consume_cond; //定义消费者条件变量
pthread_cond_t product_cond; //定义生产者条件变量

void* ConsumeStart(void* arg)//消费者消费逻辑
{
    (void)arg;
    while(1)
    {
        pthread_mutex_lock(&lock);//加锁
        while(g_val == 0) //循环判断条件
        {
            //要是没有资源可用，阻塞等逻辑
            //1.先将该PCB放到PCB等待队列当中去
            //2.再解掉之前加的锁
            //3.等待被唤醒
            pthread_cond_wait(&consume_cond, &lock);//消费者的条件变量中等待被 生产资源的生产者改变资源条件来唤醒
        }
        //有资源可用，则消费
        g_val--;
        printf("Consumer Consumeedover g_val :%d\n", g_val);
        pthread_mutex_unlock(&lock);//解锁
        //消费完之后，通知唤醒生产者条件变量里的pcb等待队列
        pthread_cond_signal(&product_cond);
    }
    return NULL;
}

void* ProductStart(void* arg)
{
    (void)arg;
    while(1)
    {
        pthread_mutex_lock(&lock);//加锁
        while(g_val == 1)//判断资源被消费
        {
            pthread_cond_wait(&product_cond, &lock);//等待消费者消费资源后唤醒
        }
        //被消费了
        g_val++;//生产
        printf("Producer  producted  g_val :%d\n", g_val);
        pthread_mutex_unlock(&lock);//解锁
        pthread_cond_signal(&consume_cond);//生产完后通知唤醒消费者条件变量里的pcb等待队列
    }
    return NULL;
}

int main()
{
    //初始化
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&consume_cond, NULL);
    pthread_cond_init(&product_cond, NULL);
    //定义生产者消费者线程
    pthread_t Consume_tid[THREADCOUNT];
    pthread_t Product_tid[THREADCOUNT];
    //pthread_t tid[2];
    int i = 0;
    int ret = -1;
    for(; i < THREADCOUNT; i++)
    {
        ret = pthread_create(&Consume_tid[i], NULL, ConsumeStart, NULL);//消费者线程
        if(ret < 0)
        {
            perror("pthread_create");
            return 0;
        }
    }

    for(i = 0; i < THREADCOUNT; i++)
    {
        ret = pthread_create(&Product_tid[i], NULL, ProductStart, NULL);//生产者线程
        if(ret < 0)
        {
            perror("pthread_create");
            return 0;
        }
    }

    for( i = 0; i < THREADCOUNT; i++)
    {
        pthread_join(Consume_tid[i], NULL);//等待线程退出回收资源
        pthread_join(Product_tid[i], NULL);
    }
    //销毁互斥锁和条件变量
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&consume_cond);
    pthread_cond_destroy(&product_cond);
    return 0;
}
