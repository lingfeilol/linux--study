#include"db_connection.h"
#include<stdio.h>
#include<stdlib.h>

//连接数据库
int connection(MYSQL** my,char* name,char* passwd,char* db)
{
    *my = mysql_init(NULL);
    mysql_real_connect(*my,"localhost",name,passwd,db,0,NULL,0);
}

//插入、删除、更改操作(执行使用sql语句)
int update(MYSQL* my,char* sql)
{
    mysql_query(my,sql); //执行操作
}

//查询 （返回结果集）
MYSQL_RES* query(MYSQL* my,char* sql)
{  
    mysql_query(my,sql);
    MYSQL_RES *res = mysql_store_result(my);
    return res;
}


