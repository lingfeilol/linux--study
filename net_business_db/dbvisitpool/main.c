#include<stdio.h>
#include<stdlib.h>
#include"db_connection.h"
int main()
{
    MYSQL* mysql=NULL;
    connection(&mysql,"root","111111","test1");
    char *sql="insert into stu value(1,'adc')";
    update(mysql,sql);
    return 0;
}
