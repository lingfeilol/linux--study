
#include<unistd.h>
#include<pthread.h>
#include <semaphore.h>
#include<stdio.h>
#include<vector>
#include<iostream>


#define SIZE 10 //宏定义队列容量大小
#define THREADCOUNT 2 //线程个数

class RingQueue 
{
  public:
    RingQueue()
      :arr_(SIZE)
  {
    CapaCity_=SIZE;
    PosRead_=0;
    PosWrite_=0;
    //生产者信号量初始化
    sem_init(&ProSem_,0,SIZE);
    //消费者信号量初始化
    sem_init(&ConSem_,0,0);
    //信号量互斥锁初始化
    sem_init(&Lock_,0,1);
  }
    ~RingQueue()
    {
      sem_destroy(&ProSem_);
      sem_destroy(&ConSem_);
      sem_destroy(&Lock_);
    }
    void Push(int & data)
    {
      sem_wait(&ProSem_);
      sem_wait(&Lock_);
      arr_[PosWrite_]=data;
      PosWrite_=(PosWrite_+1)%CapaCity_;
      sem_post(&Lock_);
      sem_post(&ConSem_);
    }
    void Pop(int* data)
    {
      sem_wait(&ConSem_);
      sem_wait(&Lock_);
      *data=arr_[PosRead_];
      PosRead_=(PosRead_+1)%CapaCity_;
      sem_post(&Lock_);
      sem_post(&ProSem_);
    }
  private:
    std::vector<int> arr_;
    size_t CapaCity_;
    int PosWrite_;
    int PosRead_;

    //信号量实现同步
    sem_t ProSem_;//生产者同步信号量
    sem_t ConSem_;//消费者
    //信号量实现互斥
    sem_t Lock_;
};
void* consumework(void* arg)
{
  RingQueue* que=(RingQueue*)arg;
  int data;
  while(1)
  {
    que->Pop(&data);
    printf("consumework [%p][%d]\n",pthread_self(),data);
    printf("haha\n");
  }
  return NULL;
}
void* productwork(void* arg)
{
  RingQueue* que=(RingQueue*)arg;
  int i=0;
  while(i)
  {
    que->Push(i);
    printf("productwork [%p][%d]\n",pthread_self(),i);
    i++;
  }
  return NULL;
}
int main()
{
  RingQueue* que=new RingQueue();
  pthread_t com_tid[THREADCOUNT],pro_tid[THREADCOUNT];
  int i=0;
  for(i=0;i<THREADCOUNT;i++)
  {
    int ret=pthread_create(&com_tid[i],NULL,consumework,(void*)que);
    if(ret<0)
    {
      printf("consume threadcreate is faild\n");
      return 0;
    }

    ret=pthread_create(&pro_tid[i],NULL,productwork,(void*)que);
    if(ret<0)
    {
      printf("prduct threadcreate is faild\n");
      return 0;
    }
  }
  for(i=0;i<THREADCOUNT;i++)
  {
    pthread_join(com_tid[i],NULL);
    pthread_join(pro_tid[i],NULL);
  }
  delete que;
  que=NULL;
  return 0;
}
