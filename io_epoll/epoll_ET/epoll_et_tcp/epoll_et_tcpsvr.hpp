#pragma once 
#include<stdio.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<stdlib.h>
#include<vector>

#include<errno.h>

#include"tcpclass.hpp"
class EpollSvr
{

public:
    EpollSvr()
    {
      epoll_fd=-1;
    }
    ~EpollSvr()
    {

    }
    bool InitSvr(int size)
    {
      //创建epoll 操作句柄
      epoll_fd = epoll_create(size);
      if(epoll_fd < 0)
      {
        perror("epoll_create");
        return false;
      }
      return true;
    }
    //对于ET模式，还需要再添加事件结构的跟上模式的指定
    bool Addevent(int fd,bool is_ET=false)
    {
        //组织事件结构、
        struct epoll_event ev;
        ev.data.fd = fd;
        if(is_ET)
            ev.events = EPOLLIN | EPOLLET;
        else 
            ev.events =EPOLLIN;

        //添加此事件结构到epoll中
        int ret=epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd,&ev);
        if(ret < 0)
        {
          perror("epoll_ctl");
          return false;
        }
        return true;
    }

    //从epoll中删除事件
    bool Delevent(int fd)
    {
      int ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL,fd ,NULL);
      if(ret < 0)
      {
        perror("epoll_ctl");
        return false;
      }
      return true;
    }

    //监视
    bool EventListen(vector<Tcpsc>* v)
    {
       struct epoll_event event_arr[10];
        int  ret=epoll_wait(epoll_fd,event_arr,sizeof(event_arr)/sizeof(event_arr[0]),-1);
        if(ret < 0)
        {
            perror("epoll_wait");
            return false;
        }

        //监视返回，即有事件触发，将其包装为 Tcpsc类对象，返回给主函数判断及使用
        for(int i=0;i < ret;i++)
        {
            Tcpsc ts;
            ts.Setfd(event_arr[i].data.fd);
            v->push_back(ts);
        }
        return true;
    }

private:
    int epoll_fd;
};
