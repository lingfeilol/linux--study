#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include"threadpool.h"
#include"net_connection.h"

int main()
{
    listen_init();
    create_epoll_init();
    epoll_process();

    return 0;
}
