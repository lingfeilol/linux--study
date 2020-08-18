#ifndef __NET_CONNECTION_H
#define __NET_CONNECTION_H

//设计网络连接层    
//定义一个结构体，将有io事件触发的描述符，对应事件，
//以及真正io读写时的读写操作（使用回调函数），读写缓冲区，
//然后再将其链起来，加入到线程池中执行业务（因为这个结构体里就包含了具体的文件描述符和函数
//这样就可以实现网络连接 与 业务数据处理 分开的框架思想


//用于描述和管理网络连接的结构体
typedef struct connection{
    int fd;     //已连接描述符
    int events;  //当前事件状态 ，EPOLLIN还是EPOLLOUT
    void (*rdhandler)(struct connection *c); //读操作回调函数
    void (*wrhandler)(struct connection *c); //写操作回调函数
    char buffer[4096];
    struct connection *deta_next; //链起来
}connection_t;

//初始化监听套接字
int listen_init();
//创建epoll操作句柄
void create_epoll_init();
//epoll_wait内核监控注册事件
void epoll_process();



#endif //__NET_CONNECTION_H
