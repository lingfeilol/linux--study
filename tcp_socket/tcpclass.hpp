#pragma once 
#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<string>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<iostream>
#include<netinet/in.h>
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
      if(sock_<0)
      {
        perror("socket");
        return false;
      }
      return true;
    }
    //绑定地址信息
    bool Bind(string& ip,uint16_t port)
    {
      struct sockaddr_in  addr;//组织成ipv4地址结构
      addr.sin_family =AF_INET;
      addr.sin_port=htons(port);
      addr.sin_addr.s_addr=inet_addr(ip.c_str());
      int ret=bind(sock_,(struct sockaddr*)&addr,sizeof(addr));
      if(ret<0)
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
      if(ret<0)
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
      if(newserverfd<0)
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
      if(ret<0)
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
      if(sendsize<0)
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
      if(recvsize<0)
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
    

  private:
    int sock_;
};


