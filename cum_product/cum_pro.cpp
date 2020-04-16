#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <cstdio>
#include <queue>

#define CAPACITY 10
#define THREADCOUNT 2
//线程安全的队列
class BlockQueue
{
    public:
        BlockQueue(size_t Capacity)
        {
            Capacity_ = Capacity;
            pthread_mutex_init(&Mutex_, NULL);
            pthread_cond_init(&ConsumeCond_, NULL);
            pthread_cond_init(&ProductCond_, NULL);
        }

        ~BlockQueue()
        {
            pthread_mutex_destroy(&Mutex_);
            pthread_cond_destroy(&ConsumeCond_);
            pthread_cond_destroy(&ProductCond_);
        }

        void Push(int& Data)
        {
            pthread_mutex_lock(&Mutex_);
            while(IsFull())
            {
                pthread_cond_wait(&ProductCond_, &Mutex_);
            }
            Queue_.push(Data);
            pthread_mutex_unlock(&Mutex_);
            pthread_cond_signal(&ConsumeCond_);
        }

        void Pop(int* Data)
        {
            pthread_mutex_lock(&Mutex_);
            while(Queue_.empty())
            {
                pthread_cond_wait(&ConsumeCond_, &Mutex_);
            }
            *Data = Queue_.front();
            Queue_.pop();
            pthread_mutex_unlock(&Mutex_);
            pthread_cond_signal(&ProductCond_);
        }
    private:
        bool IsFull()
        {
            if(Queue_.size() == Capacity_)
            {
                return true;
            }
            return false;
        }
    private:
        std::queue<int> Queue_;
        //定义队列的最大容量
        size_t Capacity_;
        //互斥
        pthread_mutex_t Mutex_;
        //同步
        pthread_cond_t ConsumeCond_;
        pthread_cond_t ProductCond_;
};

void* ConsumeStart(void* arg)
{
    BlockQueue* bq = (BlockQueue*)arg;
    while(1)
    {
        int Data;
        bq->Pop(&Data);

        printf("ConsumeStart [%p][%d]\n", pthread_self(), Data);
    }
    return NULL;
}

void* ProductStart(void* arg)
{
    BlockQueue* bq = (BlockQueue*)arg;
    int i = 0;
    while(1)
    {
        bq->Push(i);
        //cpu 时间片到了 当前线程没有打印生产信息就被挂起
        //当他再次拥有时间片的时候，从打印上一次的数据开始执行
        //程序计数器 + 上下文信息
        printf("ProductStart [%p][%d]\n", pthread_self(), i);
        i++;
    }
    return NULL;
}


int main()
{
    BlockQueue* bq = new BlockQueue(10);

    pthread_t com_tid[THREADCOUNT], pro_tid[THREADCOUNT];
    int i = 0;
    for(; i < THREADCOUNT; i++)
    {
        int ret = pthread_create(&com_tid[i], NULL, ConsumeStart, (void*)bq);
        if(ret < 0)
        {
            printf("create thread failed\n");
            return 0;
        }

        ret = pthread_create(&pro_tid[i], NULL, ProductStart, (void*)bq);
        if(ret < 0)
        {
            printf("create thread failed\n");
            return 0;
        }
    }

    for(i = 0; i < THREADCOUNT; i++)
    {
        pthread_join(com_tid[i], NULL);
        pthread_join(pro_tid[i], NULL);
    }

    delete bq;
    bq = NULL;
    return 0;
}
