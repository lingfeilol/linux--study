#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/shm.h>
#define _key 0X6666666
int main()
{
  int shmid=shmget(_key,1024,IPC_CREAT|0664);//创建共享内存
  if(shmid<0)
  {
    perror("shmget");
    return 1;
  }
  void *p= shmat(shmid,NULL,0);   //连接 权限为可读可写
  if(!p)
  {
    perror("shmat");
    return 1;
  }
  int i=0;
  sleep(2);
  while(i<10)
  {
    printf("client## %s\n",p);//读取client写入的数据
    sleep(1);
    i++;
  }
  if(shmdt(p)<0)  //脱离失败返回-1
  {
    perror("shmdt");
    return -1;
  }
  if(shmctl(shmid,IPC_RMID,NULL)<0);//销毁
  {
    perror("shmctl");
    return -1;
  }
  
  return 0;
}
