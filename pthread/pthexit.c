#include<stdio.h>
#include<unistd.h>
#include<pthread.h>

void* pthread_start(void* arg)
{
  printf("i am new pthread\n");
  sleep(5);
  pthread_exit(NULL);
  //pthread_cancel(pthread_self());终止自己
  return NULL;
}
int main()
{
  pthread_t tid;
  int ret=0;
  ret=pthread_create(&tid,NULL,pthread_start,NULL);
  if(ret<0)
  {
    perror("pthreadcreate");
    return 0;
  }
  pthread_cancel(tid);//主线程来终止 创建出来的线程
  pthread_join(tid,NULL);//回收线程资源
  
  return 0;
}
