#include"udpclass.hpp"
//对于客户端而言，命令行参数中的ip和端口号需要指定为服务端的
int main(int argc,char* argv[])
{
  if(argc!=3)
  {
    printf("arg num is error");
    return 0;
  }
  string ip(argv[1]);
  uint16_t port=atoi(argv[2]);
  Udpcs uc;
  if(!uc.CreateSocket())
  {
    return 0;
  }
  //把传进来的服务器的地址信息组织起来，以便sendto函数使用
  struct sockaddr_in dest_addr;//客户端的目标地址就是服务端的地址信息
  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(port);
  dest_addr.sin_addr.s_addr=inet_addr(ip.c_str());
  while(1)
  {
    //发送数据
    printf("client say:");
    fflush(stdout);
    string buf;
    cin>>buf;
    uc.Send(buf,&dest_addr);

    //接收数据//要保留对端的地址信息，以便回复
    struct sockaddr_in peeraddr;
    uc.Recv(&buf,&peeraddr);
  printf("server reply: %s\n",buf.c_str());

  }
  uc.Close();
  return 0;
}
