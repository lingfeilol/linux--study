#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>

typedef struct thread_info  //构造thread_info动态对象，作为线程函数参数传递给线程
{
  int thread_num; 
}THREADINFO;
void * pthread_start(void *arg)
{
  THREADINFO* it=(THREADINFO*)arg;
  int num=0;
  num=it->thread_num;
  while(1)
  {
     sleep(1);
     printf("i am pthread: %d\n",num);
  }
  free(it);
  return NULL;
}

int main()
{
  pthread_t tid;
  int ret=0;
  int i=0;
  for(i=0;i<4;i++)
  {
    THREADINFO * it=(THREADINFO*)malloc(sizeof(THREADINFO));
    it->thread_num=i;
    ret=pthread_create(&tid,NULL,pthread_start,(void*)it);
    if(ret<0)
    {
      perror("pthcreate");
      return 0;
    }
  }
  pthread_join(tid,NULL);//回收线程资源
  while(1)
  {
    sleep(1);
    printf("i am main pthread\n");
  }
  return 0;
}
