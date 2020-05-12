#include"tcpclass.hpp"

int main(int argc,char* argv[])
{
  if(argc!=3)
  {
      printf("please enter true ip and port!");
      return 0;
  }
  string ip=argv[1];
  uint16_t port=atoi(argv[2]);
  Tcpsc sc;
  if(!sc.CreateSocket())
  {
    return 0;
  }
  if(!sc.Bind(ip,port))
  {
    return 0;
  }
  if(!sc.Listen())
  {
    return 0;
  }
  Tcpsc newsersc;
    struct sockaddr_in clientaddr;//客户端connect发起连接请求，监听到了，创建新连接套接字来完成通信
    if(!sc.Accept(&clientaddr,&newsersc))//服务端响应请求时，便把客户端地址信息保留下来，而新的类指针里面保留的有新创建出来的可以进行通信的已完成连接套接字
    {
      return 0;
    }
    printf("connected cli ip:[%s],port[%d]\n",inet_ntoa(clientaddr.sin_addr),clientaddr.sin_port);
    //连接工作完成，可以进行接受和发送数据了
  while(1)
  {
    //接收数据
    string buf;
    newsersc.Recv(&buf);
    printf("client said:%s\n",buf.c_str());

    //发送数据
    printf("server reply:");
    fflush(stdout);
    cin>>buf;
    newsersc.Send(buf);
  }
  newsersc.Close();//关闭刚才通信的套接字
  sc.Close();//关闭监听套接字
  return 0;
}
