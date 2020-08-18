#include "net_connection.h"
#include"threadpool.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/epoll.h>

#if 0 
//用于描述和管理网络连接的结构体
typedef struct connection{
    int fd;     //已连接描述符
    int events;  //当前事件状态 ，EPOLLIN还是EPOLLOUT
    void (*rdhandler)(struct connection *c); //读操作回调函数
    void (*wrhandler)(struct connection *c); //写操作回调函数
    char buffer[4096];
    struct connection *deta_next; //链起来
}connection_t;
#endif

#define CONSIZE 1024
int listen_fd=0; //监听套接字
int epfd = 0; //epoll句柄
connection_t connection[CONSIZE]; //连接池 （就是一个结构体数组）
connection_t *free_connection = NULL; //连接池中空闲连接的头
threadpool_t pool; //需要线程池中的线程来处理具体业务
struct epoll_event event_arr[CONSIZE]; //epoll_wait函数（参数），返回的已就绪的event数组

void event_accept(connection_t* cur);
void read_request_handler(connection_t *cur);
void write_request_handler(connection_t *cur);
void epoll_add_event(int fd, int rd_ev,int wr_ev,int ctl_type ,connection_t* cur);



//网络连接 与 tcp通信流程一样
//初始化监听套接字
int listen_init()
{
    listen_fd=socket(AF_INET,SOCK_STREAM,0); //创建套接字
    int op=1;
    setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&op,sizeof(op)); //允许ip端口号重复使用
    //组织地址信息
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_port=htons(9000);
    addr.sin_addr.s_addr =htonl(INADDR_ANY); //允许任意ip
    int ret = bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr)); //绑定地址信息
    if(ret ==-1 )
        perror("bind"),exit(1);
    listen(listen_fd,5); //将连接套接字设置为监听套接字
    return listen_fd;
}

//从一个连接池中获取一个空闲连接
connection_t* get_connection(int fd)
{
    connection_t *cur=free_connection;
    free_connection=cur->deta_next;
    cur->fd = fd ;
    return cur;
}

//关闭一个连接
void close_connection(connection_t *c)
{
    close(c->fd);//关闭文件描述符
    c->fd = -1;
    c->deta_next = free_connection; //连接池里变为空闲连接
    free_connection = c;
}
//创建epoll操作句柄
void create_epoll_init()
{
    epfd = epoll_create1(EPOLL_CLOEXEC);
    threadpool_init(&pool,2); //初始化线程池，等待任务
    //网络连接层使用epoll监控注册的文件描述符的io事件触发，再根据事件类型，将不同的任务放入线程池的任务队列中，让线程去执行，之后数据处理完毕，再写回响应给网络层发送回去
    
    //将连接池中的元素初始化，且链接起来
    int i=CONSIZE;
    connection_t *cur = connection;  //连接池操作句柄（结构体数组，所以就是数组名）
    connection_t *next = NULL; //当前节点的下一个节点
    do
    {
        i--;
        cur[i].deta_next=next;
        cur[i].fd = -1;
        next = &cur[i];
    }while(i);
    //再让指向空闲连接的头指向头部
    free_connection = next;
    
    //给监听套接字分配连接池节点
    cur= get_connection (listen_fd);
    cur->rdhandler = event_accept ; //监听套接字对应得回调函数（监听套接字只有读事件，即处理连接请求，即调用accept函数创建新得套接字来通信）
                  //描述符  读 写  
    epoll_add_event(cur->fd,1, 0, EPOLL_CTL_ADD,cur); //将监听套接字注册到红黑树，让内核监控
}

void event_accept(connection_t* cur)
{
    int newfd= accept(cur->fd ,NULL,NULL); //等待客户端connect连接
    connection_t* newcur=get_connection(newfd); //分配（组织）连接池节点
    newcur->rdhandler = read_request_handler ;  //读回调
    newcur->wrhandler = write_request_handler ; //写回调

    epoll_add_event(newcur->fd,1,0,EPOLL_CTL_ADD,newcur);//将已连接得通信套接字注册到红黑树上，且客户端刚连上一般都是发消息过来，所以为服务端先监控读事件
}

void epoll_add_event(int fd, int rd_ev,int wr_ev,int ctl_type ,connection_t* cur)
{
    //根据具体事件来调用opll_ctl函数
    struct epoll_event ev;
   if(ctl_type == EPOLL_CTL_MOD) // 修改事件类型（一个通信描述符，有数据读，之后就会有写数据发回去，但不能直接使用epoll_CTL_MOD修改,而是先在红黑树上删掉，再反转读写事件，再添加到红黑树上）
   {
      //先将之前的删除
      epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&ev);
      //反转事件
      ev.data.ptr = cur;
      ev.events = cur->events;
      if(cur->events == EPOLLIN)
      {
          cur->events =EPOLLIN;
          ev.events =EPOLLIN;
      }
      else if(cur->events == EPOLLOUT)
      {
          cur->events = EPOLLIN;
          ev.events = EPOLLIN;
      }
      epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
   }
   else 
   {
      if(rd_ev == 1)  //监控该描述符得读事件
      {
          ev.events = EPOLLIN;
          cur->events = EPOLLIN;
      }
      else if( wr_ev ==1 )  //监控该描述符得写事件
      {
          ev.events =EPOLLOUT;
          cur->events =EPOLLOUT;
      }
      ev.data.ptr = cur; //将封装好的连接池节点 传给event结构体里得void *ptr，有了这个就可以使用里面具体得回调函数
      epoll_ctl(epfd,ctl_type,fd,&ev);
   }

}


//线程执行的业务处理
void *business_process(void* args)
{
    connection_t* cur = args;
    //具体操作可以写在这里，这里只是简单的示例：转换大小写
    int i;
    for(i=0;cur->buffer[i];i++)
    {
        if(cur->buffer[i] >='A' && cur->buffer[i] <= 'Z')
            cur->buffer[i]+=32;
       // (cur->buffer[i])^=' '; //' '的ascll码就是32
    }
    //处理完业务之后。需要写回数据，所以将当前描述符由监控读事件改为监控写事件
    epoll_add_event(cur->fd,0,0,EPOLL_CTL_MOD,cur);
}


void read_request_handler(connection_t* cur)
{
    int readsize = read(cur->fd,cur->buffer,4096);
    if(readsize <= 0)
    {
        //连接断开
        //则关闭连接池 ，且删掉红黑树中注册的节点
        epoll_ctl(epfd, EPOLL_CTL_DEL,cur->fd,0); //对端关闭，不再监控
        close_connection(cur); //归还连接池
    }
    //将读取的数据交给线程池去处理(业务处理层：解码、响应、编码) ，
    //再发送回去，而我这边返回继续监控io就绪事件
    threadpool_add_task(&pool,business_process,cur);
}

void write_request_handler(connection_t* cur)
{
    write(cur->fd,cur->buffer,strlen(cur->buffer));

    //写回数据后，再切换为监控EPOLLIN事件
    epoll_add_event(cur->fd,0,0,EPOLL_CTL_MOD,cur);
}

//epoll_wait内核监控注册事件
void epoll_process()
{
  while(1)
  {
    //循环监控
    int ret = epoll_wait(epfd,event_arr,CONSIZE,-1);
    int i;
    connection_t* cur=NULL;
    for(i=0; i < ret ;i++)
    {
        int revents = event_arr[i].events;
        cur = (connection_t* )event_arr[i].data.ptr;
        if(revents & EPOLLIN)   //为读事件，调用特定的回调函数
        {
            (cur->rdhandler)(cur);
        }
        if(revents & EPOLLOUT ) //为写事件 
        {
            (cur->wrhandler)(cur);
        }
    }
  }
}
