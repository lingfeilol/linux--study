#include"epoll_et_tcpsvr.hpp"
#define CHECK_RET(q) if(!q) {return -1;}

int main()
{
  //还是服务器端的基本逻辑 创建套接字--》绑定地址信息--》转化为监听套接字--》 加入epoll结构 --》使用epoll进行监听事件 --》返回就绪的文件描述符 --？判断是新连接还是数据到来---？若是是新连接就调用accpet函数创建新的用于通信的套接字，并将其加入epoll结构，等待事件就绪。 若是数据到来，即读取数据
  
  Tcpsc listen_ts;
  CHECK_RET(listen_ts.CreateSocket());
  string ip("0.0.0.0");
  CHECK_RET(listen_ts.Bind(ip,19999));
  CHECK_RET(listen_ts.Listen());


  EpollSvr  es;
  es.InitSvr(10);//创建epoll操作句柄
  es.Addevent(listen_ts.Getfd());//先将监控描述符添加到epoll结构中 ,并设置为ET边缘触发

  while(1)
  {
    //监控
    vector<Tcpsc> v;
    if(!es.EventListen(&v))
    {
      continue;
    }
    //返回就绪事件，判断是新连接还是数据到来
    for(size_t i = 0; i < v.size();i++)
    {
      if(v[i].Getfd() == listen_ts.Getfd())//是侦听套接字上的就绪事件，说明是新连接
      {
        //调用 accept函数创建新的套接字用于通信，并将其添加到epoll中
        struct sockaddr_in peeraddr;//对端的地址信息
        Tcpsc newts;//用于保存新创建出来的套接字
        listen_ts.Accept(&peeraddr,&newts);
        printf("新的客户端连接----->[ip]:%s,[port]:%d\n",inet_ntoa(peeraddr.sin_addr),peeraddr.sin_port);
        newts.SetNoBlock();//设置为非阻塞属性
        //再将其添加进去
        es.Addevent(newts.Getfd(),true);
      }
      else //否则，就是新数据到来，读取操作,为ET模式时，要调用阻塞接收方法  
      {
        string read_data;
        bool ret=v[i].RecvNoBlock(&read_data);
        if(!ret)
        {
            es.Delevent(v[i].Getfd());
            v[i].Close();
            continue;
        }
        printf("客户端向你说话----》：%s\n",read_data.c_str());
       
        read_data.clear();
        read_data.assign("OK!本服务器已收到！");
        v[i].SendNoBlock(read_data);
      }
    }
  }
  return 0;
}
