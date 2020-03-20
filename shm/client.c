#include<stdio.h>
#include<unistd.h>
#include<sys/shm.h>
#include<stdlib.h>
#define _key 0X6666666
int main()
{
  int shmid=shmget(_key,1024,IPC_CREAT|0664);//根据_key找到之前创建的那一块内存 
  if(shmid<0)
  {
    perror("shmget");
    return -1;
  }
  char* p=shmat(shmid,NULL,0); //连接共享内存
  int i=0;
  
  while(i<10)   //写数据
  {
    p[i]=i+'A';
    i++;
    sleep(1);
  }
  p[i]=0;
  shmdt(p);//释放
  sleep(1);


  return 0;
}
