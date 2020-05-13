#include"tcpclass.hpp"
#include<pthread.h>
void* threadstart(void* arg)
{
  Tcpsc* newsersc=(Tcpsc*)arg;
  pthread_detach(pthread_self());//分离线程，退出后由系统自动回收资源
  while(1)
  {
    //接收数据
    string buf;
    newsersc->Recv(&buf);
    printf("cli say:%s\n",buf.c_str());

    //发送数据
    printf("server reply:");
    fflush(stdout);
    cin>>buf;
    newsersc->Send(buf);
  }
  newsersc->Close();
  delete newsersc;

}
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

  while(1)
  {
    Tcpsc* newsersc=new Tcpsc();
    struct sockaddr_in clientaddr;//客户端connect发起连接请求，监听到了，创建新连接套接字来完成通信
    if(!sc.Accept(&clientaddr,newsersc))//服务端响应请求时，便把客户端地址信息保留下来，而新的类指针里面保留的有新创建出来的可以进行通信的已完成连接套接字
    {
      return 0;
    }
    printf("connected cliaddress --> ip:[%s],port[%d]\n",inet_ntoa(clientaddr.sin_addr),clientaddr.sin_port);
    
    //完成连接，创建线程去与客户端通信
    pthread_t tid;
    int ret=pthread_create(&tid,NULL,threadstart,(void*)newsersc);
    if(ret<0)
    {
      perror("pthread_create");
      return 0;
    }
  }
  return 0;
}
