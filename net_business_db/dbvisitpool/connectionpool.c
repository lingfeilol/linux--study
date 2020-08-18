#include"connectionpool.h"
#include<stdlib.h>
#include<unistd.h>

#if 0
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

#endif 

void connection_pool_init(connectionpool* pool,char* name,char* passwd,char* db,int max_size)
{
    pool->name = name ;
    pool->passwd =passwd;
    pool->db=db;
    pool->max_size =max_size ;
    pool->cur_size =0;
    pool->head = pool->tail =NULL;
    pthread_cond_init(&(pool->cond),NULL);
    pthread_mutex_init(&(pool->mutex),NULL);
}

void connection_pool_push(connectionpool_t* pool,char* sql)
{
    pthread_mutex_lock(&(pool->mutex));
    if( pool->head !=NULL )
    {
        //有空闲连接,直接使用
        node_t* tmp = pool->head;
        pool->head = tmp->next;
        //取下连接节点，执行数据库CRUD操作
        pthread_mutex_unlock(&(pool->mutex));
        update(tmp->conn_mysql,sql);
        pthread_mutex_lock(&(pool->mutex));
        
        //使用完后，再放回连接池
        tmp->next =pool->head ;
        pool->head=tmp;
        pthread_cond_signal(&(pool->cond));
    }
    else if( pool->cur_size < pool->max_size )
    {
        //当前使用的连接个数少于最大个数，则可以创建新的连接
        node_t *newconn = (node_t*)malloc(sizeof(node_t));
        connection(&(newconn->conn_mysql),pool->name,pool->passwd,pool->db); 
        //新连接，直接执行sql
        pthread_mutex_unlock(&(pool->mutex));
        update(newconn->conn_mysql,sql);
        pthread_mutex_lock(&(pool->mutex));

        //使用完后，再插入连接池
        newconn->next = pool->head;
        pool->head = newconn;
        pool->cur_size++;
        pthread_cond_signal(&(pool->cond));
    }
    else 
    {
        //否则，只能等待
        while(pool->head ==NULL)
        {
            pthread_cond_wait(&(pool->cond),&(pool->mutex));
        }
        //被唤醒，拿资源去执行
        node_t* tmp = pool->head;
        pool->head = tmp->next;
        pthread_mutex_unlock(&(pool->mutex));
        update(tmp->conn_mysql,sql);
        pthread_mutex_lock(&(pool->mutex));
        
        //使用完后，再放回连接池
        tmp->next =pool->head ;
        pool->head=tmp;
        pthread_cond_signal(&(pool->cond));
    }
    pthread_mutex_unlock(&(pool->mutex));
}
