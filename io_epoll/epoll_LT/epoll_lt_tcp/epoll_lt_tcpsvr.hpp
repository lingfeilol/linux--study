#pragma once 

#include "tcpclass.hpp"
#include<vector>
#include<stdio.h>
#include<unistd.h>
#include<sys/epoll.h>


class EpollSever
{

  public:
    EpollSever()
    {
      epoll_fd=-1;
    }
    ~EpollSever()
    {}
    bool init_create(int size)
    {
      epoll_fd = epoll_create(size);
      if(epoll_fd < 0)
      {
        return false;
      }
      return true;
    }

    bool Add_events(int fd)//往epoll结构里添加要监视的事件
    {
       struct epoll_event ev;
       ev.data.fd=fd;
       ev.events=EPOLLIN;
      int ret= epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&ev);
      if(ret < 0)
      {
         perror("epoll_ctl");
         return false;
      }
     return true;
    }

    bool Del_events(int fd)//删除事件
    {
      int ret=epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,NULL);
      if(ret < 0)
      {
        perror("epoll_ctl");
        return false;
      }
      return true;
    }

    bool Epoll_Listen(vector<Tcpsc>* v)
    {
      struct epoll_event event_arr[10];
      size_t ret=epoll_wait(epoll_fd,event_arr,sizeof(event_arr)/sizeof(event_arr[0]),-1);
      if(ret < 0)
      {
        perror("epoll_wait");
        return false;
      }
      else if(ret == 0)
      {
        printf("timeout!");
        return false;
      }
      if(ret > sizeof(event_arr)/sizeof(event_arr[0])) //防止数组越界
      {
        ret = sizeof(event_arr)/sizeof(event_arr[0]);
      }
      for(size_t i= 0;i < ret; i++)//将就绪的IO事件封装到tcp类中，由那边具体使用
      {
        Tcpsc tc;
        tc.Setfd(event_arr[i].data.fd);
        v->push_back(tc);
      }
      return true;
    }

  private:
    int epoll_fd;//epoll操作句柄

};
