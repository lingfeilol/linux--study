#include"tcpclass.hpp"
#include<sys/wait.h>
#include<signal.h>


void sigback(int signo)
{
  (void)signo;
  wait(NULL);
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

  signal(SIGCHLD,sigback);//进程信号SIGCHLD，子进程退出默认给父进程发出的信号，但是默认为不关心，所以自定义信号处理函数，避免僵尸进程
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
    Tcpsc newsersc;
    struct sockaddr_in clientaddr;//客户端connect发起连接请求，监听到了，创建新连接套接字来完成通信
    if(!sc.Accept(&clientaddr,&newsersc))//服务端响应请求时，便把客户端地址信息保留下来，而新的类指针里面保留的有新创建出来的可以进行通信的已完成连接套接字
    {
      //return 0;因为要处理多进程，所以不能直接退出
      continue;
    }
    printf("connected cliaddress --> ip:[%s],port[%d]\n",inet_ntoa(clientaddr.sin_addr),clientaddr.sin_port);

    //连接工作完成，创建子进程去完成与客户端的通信
    int pid=fork();
    if(pid<0)
    {
      perror("fork");
      exit(0);
    }
    else if(pid==0)
    {
      //child 子进程循环通信
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
      newsersc.Close();//子进程关闭已连接套接字，其实子进程退出也是会自动关闭打开的文件描述符的
      exit(0);
    }
    else 
    {
      //父进程
      newsersc.Close();//由于父进程主要用于监听连接请求，所以accept创建的已连接套接字是不会使用的，是要先自己手动关闭的
    }
  }
    return 0;
}
