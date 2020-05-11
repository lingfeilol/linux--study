#pragma once 
#include<stdio.h>
#include<unistd.h>
#include<string>
#include<cstring>
#include<cstdlib>
#include<iostream>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>

using namespace std;

class Udpcs
{
  public:
    Udpcs()
    {
      sock_=-1;
    }
    ~Udpcs()
    {

    }
    //创建套接字
    bool CreateSocket()
    {
      sock_=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
      if(sock_<0)
      {
        perror("socket");
        return false;
      }
      return true;
    }

     //绑定地址信息 服务器端调用。客户端由内核自动分配
    bool Bind(string& ip,uint16_t port)
    {
      struct sockaddr_in addr;
      addr.sin_family =AF_INET;
      addr.sin_port=htons(port);
      addr.sin_addr.s_addr=inet_addr(ip.c_str());//将点分十进制的ip转换成二进制的网络字节序
      int ret=bind(sock_,(struct sockaddr*)&addr,sizeof(addr));
      if(ret<0)
      {
        perror("bind");
        return false;
      }
      return true;

    }
    //3.发送数据
    //udp只需要知道对方的ip及端口号，就可以不用连接就发送消息
    //dest_addr 目的地址
    bool Send(string& data,struct sockaddr_in* dest_addr)//结构体里保存的ip和端口号等
    {
      int sendsize=sendto(sock_,data.c_str(),data.size(),0,(struct sockaddr*)dest_addr,sizeof(struct sockaddr_in));
      if(sendsize<0)
      {
        perror("send");
        return false;
      }
      return true;
    }
    //4.接收数据
    //src_addr 原地址
    bool Recv(string* buf,struct sockaddr_in* src_addr)
    {
      char tmp[1024];//临时接收和保存数据
      memset(tmp,'\0',sizeof(tmp));
      socklen_t socklen=sizeof(struct sockaddr_in);
      int recvsize=recvfrom(sock_,tmp,sizeof(tmp)-1,0,(struct sockaddr*)src_addr,&socklen);
      if(recvsize<0)
      {
        perror("recvform");
        return false;
      }
      (*buf).assign(tmp,recvsize);
      return true;
    }
   // 5.关闭套接字
      void Close()
      {
        close(sock_);
        sock_=-1;
      }
  private:

    int sock_;//定义描述符
    
};
