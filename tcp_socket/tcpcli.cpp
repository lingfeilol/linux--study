
#include"tcpclass.hpp"

int main(int argc,char* argv[])
{
  if(argc!=3)
  {
    printf("please enter true server_ip and port!");
      return 0;
  }
  string ip=argv[1];
  uint16_t port=atoi(argv[2]);
  Tcpsc sc;
  if(!sc.CreateSocket())
  {
    return 0;
  }
  if(!sc.Connect(ip,port))
  {
    return 0;
  }
  //连接完成，开始收发数据
  while(1)
  {
    //发送数据
    printf("cli say:");
    fflush(stdout);
    string buf;
    cin>>buf;
    sc.Send(buf);
    
    //接收服务端回复的数据
    sc.Recv(&buf);
    printf("server reply:%s\n",buf.c_str());
  }
  sc.Close();//其实进程结束后会自动关闭描述符的
  return 0;
}
