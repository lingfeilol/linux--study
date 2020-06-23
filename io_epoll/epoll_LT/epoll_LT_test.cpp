#include<stdio.h>
#include<unistd.h>
#include<sys/epoll.h>

int main()
{

  //1.创建epoll操作句柄
  int epollfd=epoll_create(10);
  if(epollfd < 0)
  {
    perror("epoll_create");
    return 0;
  }
  //2.组织添加事件结构
  struct  epoll_event ev;
  ev.data.fd=0;
  ev.events=EPOLLIN;
  epoll_ctl(epollfd,EPOLL_CTL_ADD,0,&ev);

  //3.监视
  while(1)
  {

    struct epoll_event event_arr[10];//事件结构数组
    int ret=epoll_wait(epollfd,event_arr,10,5000);//监视
    if(ret < 0)
    {
      perror("epoll_wait");
      continue;
    }
    else if(ret == 0)
    {
      printf("timeout\n");
      continue;
    }
    //有事件触发，监视函数返回，读取数据
    //epoll默认为水平触发，只要缓冲区不为空，监视函数就会一直返回，读取数据
    for(int i = 0; i < ret; i++)
    {
      if(event_arr[i].events == EPOLLIN)
      {
        //char buf[1024]={0};
        char buf[3]={0};//将接收缓冲区容量设置为3，测试水平触发的工作流程
        read(0,buf,sizeof(buf)-1);
        printf("stdin: %s\n",buf);
      }
    }
  }


  return 0;
}

