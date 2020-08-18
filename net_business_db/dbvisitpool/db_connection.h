#ifndef __DB_CONNECTION_H_
#define __DB_CONNECTION_H_

#include<mysql/mysql.h>
//操作数据库的相关接口封装

//连接数据库
int connection(MYSQL** my,char* name,char* passwd,char* db);

//插入、删除、更改操作(执行使用sql语句)
int update(MYSQL* my,char* sql);

//查询 （返回结果集）
MYSQL_RES* query(MYSQL* my,char* sql);

#endif // __DB_CONNECTION_H_

