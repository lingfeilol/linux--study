#ifndef _CONNECTIONPOOL_H_
#define _CONNECTIONPOOL_H_

#include"db_connection.h"
#include<pthread.h>

//连接池节点，也用链表组织管理
typedef struct node
{
    MYSQL* conn_mysql;
    struct node* next;
}node_t;

typedef struct connectionpool{
    char* name;                //用户名
    char* passwd;              //密码
    char* db;                  //数据库名
    int max_size;              //最大连接数
    int cur_size;              //当前连接数
    node_t* head;              //已连接的头
    node_t* tail;              //尾
    pthread_cond_t cond;       //线程取连接时需要线程安全
    pthread_mutex_t mutex;
}connectionpool_t;

void connection_pool_init(connectionpool_t* pool,char* name,char* passwd,char* db,int max_size);

void connection_pool_push(connectionpool_t * pool,char* sql);
#endif// _CONNECTIONPOOL_H_
