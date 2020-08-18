
#include<cstdio>
#include<iostream>
#include<string>
#include<mysql/mysql.h>
#include<jsoncpp/json/json.h>
#include<mutex>
using namespace std;

/*
 使用C语言mysq|的API接口实现mysq|数据库的访问:实际上就是编写一个mysq|客户端
1.初始化mysq|句柄
2.连接mysq|服务器
3.设置客户端的字符编码集
4.语句执行(增删改查)
5.增删改--只要知道执行结果如何就能判断成功与否;
  对于查询执行成功，还得去获取数据
  查询流程: 1. 获取结果集。2. 遍历结果集获取各条数据。3. 释放结果集
6.关闭数据库
*/



#define MYSQL_HOST   "127.0.0.1"
#define MYSQL_DB     "blog_system"
#define MYSQL_USER   "root"
#define MYSQL_PASS   "111111"

namespace blog_system 
{
    std::mutex _mutex; //确保查询操作 和 保存查询结果集之间 的死锁问题，让其为原子操作
    //静态函数
    static MYSQL* MysqlInit()  //初始化操作句柄，且连接到数据库
    {
      MYSQL* mysql=NULL;
      //1.初始化
      mysql=mysql_init(NULL); //传NULL则自动分配
      if(mysql == NULL)
      {
        printf("mysql_init error\n");
        return NULL;
      }
      //2.连接数据库，需要参数：主机地址、用户、数据库名、密码、端口、套接字文件、客户端标志
      if(mysql_real_connect(mysql,MYSQL_HOST,MYSQL_USER,MYSQL_PASS,MYSQL_DB,0,NULL,0) == NULL)
      {
          printf("mysql_real_connect: failed! :%s \n",mysql_error(mysql));
          mysql_close(mysql);
          return NULL;
      }
      //3.设置字符编码集
      if(mysql_set_character_set(mysql,"utf8")!=0) //成功返回0，失败返回非0
      {
          printf("mysql_set_character_set:faild! : %s\n",mysql_error(mysql));
          mysql_close(mysql);
          return NULL;
      }
      return mysql;//初始化完毕
    }

    static void MysqlRelsase(MYSQL* mysql) //释放句柄资源
    {
        if(mysql)
           mysql_close(mysql);
        return ;
    }
    
    //常用的执行sql语句的函数，所以也设置为static
    static bool MysqlQuery(MYSQL* mysql,const string& sql)
    {
        int ret=mysql_query(mysql,sql.c_str());
        if(ret != 0)
        {
            printf("mysql_query faild! sql:%s,faild info:%s\n",sql.c_str(),mysql_error(mysql));
            return false;
        }
        return true;
    }

    //标签表的增删查改接口封装
    class TableTag
    {
      public:
        TableTag(MYSQL* mysql):_mysql(mysql)
        {}

        bool Insert (const Json::Value& tag)
        {
#define INSERT_TAG "insert into table_tag values(NULL,'%s');"  ///组织的sql语句
            char tmp[4096];
            sprintf(tmp,INSERT_TAG,tag["name"].asCString());
            int ret = MysqlQuery(_mysql,tmp);
            if(ret == false) 
            {
              return false; //插入失败
            }
            return true;
        }

        bool Delete (int tag_id)
        {
#define DELETE_TAG "delete from table_tag where id=%d;"  ///组织的sql语句
            char tmp[4096];
            sprintf(tmp,DELETE_TAG,tag_id);
            int ret = MysqlQuery(_mysql,tmp);
            if(ret == false) 
            {
                return false; 
            }
            return true;
        }

        bool Update(int tag_id,const Json::Value& tag)
        {
#define UPDATE_TAG "update table_tag set name='%s' where id=%d;"
            char tmp[4096];
            sprintf(tmp,UPDATE_TAG,tag["name"].asCString(),tag_id);  //json对象也重载了[]
            int ret = MysqlQuery(_mysql,tmp);
            if(ret == false)
            {
                return false;
            }
            return true;
        }

        bool GetAll(Json::Value* tags) //输出型参数，保存查询结果
        {
#define SELECT_TAG_ALL "select * from table_tag;"
          //加锁
          std::lock_guard<std::mutex> lkmutex(_mutex);
          //直接执行，不用组织了
            int ret=MysqlQuery(_mysql,SELECT_TAG_ALL);
            if(ret == false)
                return false;
            //但是要获取结果集，并遍历保存到本地。且结果集是动态开辟，最后要释放，避免内存泄露
            MYSQL_RES* res = mysql_store_result(_mysql);
            //接着获取结果集的行数，来遍历
            int rows=mysql_num_rows(res);
            if(rows <= 0)
            {
                printf("table_tag is empty!\n");
                mysql_free_result(res);//注意要释放结果集资源
                return false;
            }
            for(int i=0;i < rows; ++i)
            { 
                MYSQL_ROW row = mysql_fetch_row(res);//自动一行一行的遍历字符集，把每一行的数据保存到MYSQL_ROW类型的变量中，在横向遍历存储结果
                Json:: Value tag;
                tag["id"]= stoi(row[0]);
                tag["name"]=row[1];
                tags->append(tag); //写入到输出型参数中
            }
            mysql_free_result(res);
            return true;
        }
        bool GetOne(int tag_id,Json::Value* tag)
        {
#define SELECT_TAG_ONE "select name from table_tag where id=%d;"
            char tmp[4096];
            sprintf(tmp,SELECT_TAG_ONE,tag_id);
            std::lock_guard<std::mutex> lkmutex(_mutex);
            int ret=MysqlQuery(_mysql,tmp);
            if(ret == false)
            {
                return false;
            }
            MYSQL_RES * res =mysql_store_result(_mysql);
            int rows=mysql_num_rows(res);
            if(rows != 1) //注意是 查询单条数据
            {
                  printf("select one faild\n");
                  mysql_free_result(res);
                  return false;
            }
            for(int i=0;i < rows;++i)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                //因为只有一条，所以直接赋值即可
                (*tag)["id"]=tag_id; 
                (*tag)["name"]=row[0];
            }
            mysql_free_result(res);
            return true;
        }
      private: 
          MYSQL* _mysql;//数据库操作句柄
    };

    //用户表的增删查改
    class TableUser
    {
      public:
        TableUser(MYSQL* mysql):_mysql(mysql)
        {}
        bool Insert(const Json::Value& user)
        {
#define INSERT_USER "insert into table_user values(NULL,'%s');"
            char tmp[4096];
            sprintf(tmp,INSERT_USER,user["name"].asCString());
            int ret = MysqlQuery(_mysql,tmp);
            if(ret == false)
            {
                return false;
            }
            return true;
        }
        bool Delete(int user_id)
        {
#define DELETE_USER "delete from table_user where id=%d;"
            char tmp[4096];
            sprintf(tmp,DELETE_USER,user_id);
            int ret= MysqlQuery(_mysql,tmp);
            if(ret == false)
                return false;
            return true;
        }
        bool Update(int user_id,const Json::Value& user )
        {
#define UPDATE_USER "update table_user set name='%s' where id=%d;"
            char tmp[4096];
            sprintf(tmp,UPDATE_USER,user["name"].asCString(),user_id);
            int ret =MysqlQuery(_mysql,tmp);
            if(ret ==false)
                return false;
            return true;
        }
        bool GetAll(Json::Value * users)
        {
#define SELECT_USER_ALL "select * from table_user;"
            std::lock_guard<std::mutex> lkmutex(_mutex);
            int ret =MysqlQuery(_mysql,SELECT_USER_ALL);
            if(ret == false)
                return false;
            MYSQL_RES * res =mysql_store_result(_mysql);
            int rows = mysql_num_rows(res);
            if(rows <= 0)
            {
                printf("table_user is empty()\n");
                mysql_free_result(res);
                return false;
            }
            for(int i=0;i<rows;i++)
            {
                MYSQL_ROW row=mysql_fetch_row(res);
                Json::Value user;
                user["id"]=stoi(row[0]);
                user["name"]= row[1];
                users->append(user);
            }
            mysql_free_result(res);
            return true;
        }
        bool GetOne(int user_id,Json::Value* user)
        {
#define SELECT_USER_ONE "select name from table_user where id=%d;"
            char tmp[4096];
            sprintf(tmp,SELECT_USER_ONE,user_id);
            std::lock_guard<std::mutex> lkmutex(_mutex);
            int ret = MysqlQuery(_mysql,tmp);
            if(ret == false)
                return false;
            MYSQL_RES * res =mysql_store_result(_mysql);
            int rows = mysql_num_rows(res);
            if(rows != 1)
            {
                printf("not this one info\n");
                mysql_free_result(res);
                return false;
            }
            for(int i= 0;i < rows;i++)
            {
                MYSQL_ROW row =mysql_fetch_row(res);
                (*user)["id"]=user_id;
                (*user)["name"]=row[0];
            }
            mysql_free_result(res);
            return true;
        }

      private:
        MYSQL* _mysql;  
    };

    //博客表的增删查改
    class TableBlog
    {
      public:
        TableBlog(MYSQL* mysql): _mysql(mysql)
        {}
        bool Insert(const Json::Value& blog)
        {
        //博客字段： id tag_id user_id title content ctime
#define INSERT_BLOG "insert into table_blog values(NULL,%d,%d,%s,%s,now());"
            char tmp[4096];
            sprintf(tmp,INSERT_BLOG,blog["tag_id"].asInt(),blog["user_id"].asInt(),blog["title"].asCString(),blog["content"].asCString());
            int ret = MysqlQuery(_mysql,tmp);
            if(ret == false) 
                return false;
            return true;
        }

        bool Delete(int blog_id)
        {
#define DELETE_BLOG "delete from table_blog where id=%d;"
            char tmp[4096];
            sprintf(tmp,DELETE_BLOG,blog_id);
            int ret =MysqlQuery(_mysql,tmp);
            if(ret ==false)
                return false;
            return true;
        }
        bool Update(int blog_id ,const Json:: Value& blog)
        {
          //修改博客，只能自己修改，且修改 标签和内容
#define UPDATE_BLOG "update table_blog set title='%s' content='%s' where id=%d;"
            char tmp[4096];
            sprintf(tmp,UPDATE_BLOG,blog["title"].asCString(),blog["content"].asCString(),blog_id);
            int ret =MysqlQuery(_mysql,tmp);
            if(ret == false)
                return false;
            return true;
        }
        bool GetAll(Json::Value* blogs)
        {
#define SELECT_BLOG_ALL "select * from table_blog;"
            std::lock_guard<std::mutex> lkmutex(_mutex);
            int ret = MysqlQuery(_mysql,SELECT_BLOG_ALL);
            if(ret == false)
                return false;
            MYSQL_RES* res = mysql_store_result(_mysql);
            int rows= mysql_num_rows(res);
            if(rows <= 0)
            {
                printf("table_blog is empty\n");
                mysql_free_result(res);
                return false;
            }
            for(int i=0;i < rows ;++i)
            {
                MYSQL_ROW row =mysql_fetch_row(res);
                Json:: Value blog;
                blog["id"]=stoi(row[0]);
                blog["tag_id"]=stoi(row[1]);
                blog["user_id"]=stoi(row[2]);
                blog["title"]=row[3];
                blog["content"]=row[4];
                blog["ctime"]=row[5];
                blogs->append(blog);
            }
            mysql_free_result(res);
            return true;
        }
        //获取指定标签的博客
        bool GetTagAll(int tag_id,Json::Value* blogs)
        {
#define SELECT_TAG_BLOG_ALL "select * from table_blog where tag_id=%d;"
            char tmp[4096];
            sprintf(tmp,SELECT_TAG_BLOG_ALL,tag_id);
            std::lock_guard<std::mutex> lkmutex(_mutex);
            int ret = MysqlQuery(_mysql,tmp);
            if(ret == false)
                return false;
            MYSQL_RES* res=mysql_store_result(_mysql);
            int rows = mysql_num_rows(res);
            if(rows <= 0)
            {
                printf("not this tag_id's blogs\n");
                mysql_free_result(res);
                return false;
            }
            for(int i=0;i < rows ;++i)
            {
                MYSQL_ROW row = mysql_fetch_row(res);
                Json::Value blog;
                blog["id"]=stoi(row[0]);
                blog["tag_id"]=stoi(row[1]);
                blog["user_id"]=stoi(row[2]);
                blog["title"]=row[3];
                blog["content"]=row[4];
                blog["ctime"]=row[5];
                blogs->append(blog);
            }
            mysql_free_result(res);
            return true;
        }
        //获取指定用户的博客
        bool GetUserAll(int user_id,Json::Value* blogs)
        {
#define SELECT_USER_BLOG_ALL "select * from table_blog where user_id=%d;"
            char tmp[4096];
            sprintf(tmp,SELECT_USER_BLOG_ALL,user_id);
            std::lock_guard<std::mutex> lkmutex(_mutex);
            int ret =MysqlQuery(_mysql,tmp);
            if(ret ==false) 
                return false;
            MYSQL_RES* res =mysql_store_result(_mysql);
            int rows = mysql_num_rows(res);
            if(rows <= 0)
            {
                printf("not this user_id's blogs\n");
                mysql_free_result_start(res);
                return false;
            }
            for(int i=0;i < rows; ++i)
            {
                MYSQL_ROW row =mysql_fetch_row(res);
                Json::Value blog;
                blog["id"]=stoi(row[0]);
                blog["tag_id"]=stoi(row[1]);
                blog["user_id"]=stoi(row[2]);
                blog["title"]=row[3];
                blog["content"]=row[4];
                blog["ctime"]=row[5];
                blogs->append(blog);
            }
            mysql_free_result(res);
            return true;
        }

        //获取指定博客
        bool GetThisOne(int blog_id,Json::Value * blog)
        {
#define SELECT_BLOG_ONE "select * from table_blog where id=%d;"
            char tmp[4096];
            sprintf(tmp,SELECT_BLOG_ONE,blog_id);
            std::lock_guard<std::mutex> lkmutex(_mutex);
            int ret = MysqlQuery(_mysql,tmp);
            if(ret == false)
                return false;
            MYSQL_RES* res =mysql_store_result(_mysql);
            int rows=mysql_num_rows(res);
            if(rows != 1)
            {
                printf("this blog_id's blog not empty\n ");
                mysql_free_result(res);
                return false;
            }
            for(int i=0;i < rows ;i++)
            {
                MYSQL_ROW row =mysql_fetch_row(res);
                (*blog)["id"]=blog_id;
                (*blog)["tag_id"]=stoi(row[1]);
                (*blog)["user_id"]=stoi(row[2]);
                (*blog)["title"]=row[3];
                (*blog)["content"]=row[4];
                (*blog)["ctime"]=row[5];
            }
            mysql_free_result(res);
            return true;
        }
      private:
        MYSQL* _mysql;
    };

}
