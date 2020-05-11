#include"udpclass.hpp"

int main(int argc,char* argv[])
{
  if(argc!=3)
  {
    printf("arg num is error!\n");
    return 0;
  }
  //从参数列表里传入的ip和端口号
  string ip=argv[1];
  uint16_t port=atoi(argv[2]);
  Udpcs us;
  if(!us.CreateSocket() )
  {
    return 0;
  }
  if(!us.Bind(ip,port))
  {
    return 0;
  }
  //绑定好了地址信息，准备接收和回复消息
  while(1)
  {
    //接收数据
    string buf;
    struct sockaddr_in peeraddr;//对端的地址信息
    us.Recv(&buf,&peeraddr);//都是出参，会保留对端的地址信息，之后回复会用
    printf("client say:%s\n",buf.c_str());

    //接着回复数据
    printf("server reply:");
    fflush(stdout);
    cin>>buf;
    us.Send(buf,&peeraddr);

  }
  //对话结束后，关闭描述符
  us.Close();
  return 0;
}
