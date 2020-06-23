#pragma once 
#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<string>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<iostream>
#include<netinet/in.h>
#include<sys/types.h>
#include<fcntl.h>
using namespace std;

class Tcpsc
{
  public:
    Tcpsc()
    {
      sock_=-1;
    }
    ~Tcpsc()
    {

    }
    //创建套接字
    bool CreateSocket()
    {
      sock_=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      if(sock_ < 0)
      {
        perror("socket");
        return false;
      }
      int opt=1;
      setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));//地址复用

      return true;
    }
    //绑定地址信息
    bool Bind(std::string& ip,uint16_t port)
    {
      struct sockaddr_in  addr;//组织成ipv4地址结构
      addr.sin_family =AF_INET;
      addr.sin_port=htons(port);
      addr.sin_addr.s_addr=inet_addr(ip.c_str());
      int ret=bind(sock_,(struct sockaddr*)&addr,sizeof(addr));
      if(ret < 0)
      {
        perror("bind");
        return false;
      }
      return true;
    }
    //监听
    bool Listen(int backlog=5)
    {
      int ret=listen(sock_,backlog);
      if(ret < 0)
      {
        perror("listen");
        return false;
      }
      return true;
    }

    //accept 服务器获取连接
    //bool Accept(struct sockaddr_in* peeraddr,int* newfd)
    //peeraddr :出参。保存的是客户端的地址信息，newfd：出参，表示完成连接的可以进行通信的新创建出来的套接字描述符
    
    bool Accept(struct sockaddr_in* peeraddr,Tcpsc* newsc)//这里用一个类的实例化指针，把数据传出去
    {
      socklen_t addrlen=sizeof(struct sockaddr_in);//记录地址信息长度
      int newserverfd=accept(sock_,(struct sockaddr*)peeraddr,&addrlen);
      if(newserverfd < 0)
      {
        perror("accept");
        return false;
      }
      newsc->sock_=newserverfd;//传出去新创建出来的用来通信的套接字
      return true;
    }
    //connect 客户端调用来连接服务端
    bool Connect(string& ip,uint16_t port)
    {
      struct sockaddr_in addr;//还是先组织服务端地址信息
      addr.sin_family =AF_INET;
      addr.sin_port=htons(port);
      addr.sin_addr.s_addr=inet_addr(ip.c_str());
      int ret=connect(sock_,(struct sockaddr*)&addr,sizeof(addr));
      if(ret < 0)
      {
        perror("connect");
        return false;
      }
      return true;
    }
    //因为是已经建立连接了的，所以参数就只是数据，和已完成连接的可以进行通信的socket套接字
    //发送数据
    bool Send(string& data)
    {
      int sendsize=send(sock_,data.c_str(),data.size(),0);
      if(sendsize < 0)
      {
        perror("sned");
        return false;
      }
      return true;
    }
    //接收数据
    bool Recv(string* data)//出参，保留信息
    {
      char buf[1024]={0};
      int recvsize=recv(sock_,buf,sizeof(buf)-1,0);
      if(recvsize < 0)
      {
        perror("recv");
        return false;
      }
      else if(recvsize==0)//对端已关闭close
      {
        printf("peer is close connect");
        return false;
      }
      (*data).assign(buf,recvsize);//赋值给传出型参数
      return true;
    }
    //关闭套接字
    void Close()
    {
        close(sock_);
        sock_=-1;
    }
    
    int Getfd()
    {
      return sock_;
    }
    void Setfd(int fd)
    {
      sock_=fd;
    }
    //ET模式下的非阻塞接收和非阻塞发送
    //利用while循环 ，将数据保存到出参data里
    bool RecvNoBlock(string* data )
    {
        while(1)
        {
            //sockfd_ 不是侦听套接字，而是已连接的用于通信的套接字描述符了
            //sockfd_ 已经被设置加上了非阻塞属性，所以在判断返回值时候，需要注意 缓冲区为空（正好被接收完）的时候，recv函数返回有EAGAIN或者EWOULDBLOCK的情况产生
            char buf[3]={0};
            ssize_t readsize = recv(sock_,buf,sizeof(buf)-1,0);
            if(readsize < 0)
            {
              if(errno == EAGAIN || errno == EWOULDBLOCK)
              {
                  break;
              }
              perror("recv");
              return false;
              
            }
            else if(readsize == 0)
            {
                printf("对端关闭了连接！");
                return false;
            }
            *data += buf;
            if(readsize < (ssize_t)sizeof(buf)-1)
            {
              break;
            }
        }
        return true;
    }
    
    //非阻塞发送
    bool SendNoBlock(string& buf)//将传进来的数据发送出去
    {
      //使用指针和字节数 来确保数据全部发送完
      ssize_t pos=0; //记录当前写到的位置
      ssize_t lensize = buf.size(); //记录剩余字节数
      while(1)
      {
        //对于非阻塞IO写入，如果tcp的发送缓冲区已经满了，则写操作也会返回 错误码提示 
          ssize_t sendsize = send(sock_, buf.data()+pos,lensize, 0);
          if(sendsize < 0 )
          {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
            {
              //即使发送缓冲区满了，可能也没有把所有数据全部写入，所以继续重新写入
                continue;
            }
            perror("send");
            return false;
          }
          //更新指针位置 和 剩余字节数   即加减实际发送字节数
          pos += sendsize; 
          lensize -= sendsize;
          //推出条件，即真正写完了
          if(lensize  <= 0)
          {
              break;
          }
      }
      return true;
    }

    //将文件描述符设置为非阻塞属性 ET模式下
    void SetNoBlock()
    {
        int fl =fcntl(sock_,F_GETFL);
        if(fl < 0)
        {
          perror("fcntl");
          return ;
        }
        fcntl(sock_,F_SETFL,fl| O_NONBLOCK);
    }
  private:
    int sock_;
};


