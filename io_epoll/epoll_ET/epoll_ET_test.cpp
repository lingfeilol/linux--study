#include<stdio.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<fcntl.h>
#include<string>
#include<errno.h>
using namespace std;

void SetfdNoBlock(int fd)//设置非阻塞属性
{
  int fl=fcntl(fd,F_GETFL);
  if(fl < 0)
  {
    perror("fcntl");
    return ;
  }
  fcntl(fd,F_SETFL, fl|O_NONBLOCK);
}

int main()
{
  //1.将标准输入文件描述符设置为非阻塞属性（用于边缘触发ET模式只通知一次，所以必须使用循环读，来判断是否读取完 整条数据）
  SetfdNoBlock(0);
  //2.创建epoll结构 返回操作句柄
  int epollfd=epoll_create(10);
  if(epollfd < 0)
  {
    perror("epoll_create");
    return 0;
  }
  //3.组织事件结构，再将其加入监视
  struct epoll_event ev;
  ev.data.fd=0;
  ev.events=EPOLLIN |EPOLLET ; //ET模式
  epoll_ctl(epollfd,EPOLL_CTL_ADD,0,&ev);

  //4.监视
  while(1)
  {
    epoll_event event_arr[10];
    int ret=epoll_wait(epollfd,event_arr,10,-1);
    if(ret < 0)
    {
      perror("epoll_wait");
      continue;
    }
    //有IO事件触发，监视函数检测到后返回触发个数
    for(int i = 0;i < ret; i++)
    {
      if(event_arr[i].events == EPOLLIN )
      {
        string read_ret;
        while(1)//由于ET模式只会通知一次，所以必须加循环将缓冲区的所有数据读完
        {
            char buf[3]={0};
            ssize_t readsize = read(0,buf,sizeof(buf)-1);
            if(readsize < 0)
            {
                 // 对于非阻塞 IO 读数据, 如果 接受缓冲区为空, 就会返回错误
                 // 错误码为 EAGAIN 或者 EWOULDBLOCK , 需要重试
                if(errno == EAGAIN || errno == EWOULDBLOCK )
                {
                  goto overend;
                }
                perror("read");
                return 0;
            }
            read_ret+=buf;
            if(readsize <(ssize_t)sizeof(buf)-1)
            {
overend:
              printf("stdin: %s\n",read_ret.c_str());
              break;
            }

        }
      }
    }
  }
  return 0;
}
