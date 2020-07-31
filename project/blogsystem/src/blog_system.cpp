#include "db.hpp"
#include"httplib.h"
#include<pthread.h>
#define WWWROOT "./www"
void test()
{
    MYSQL* mysql_fd = blog_system::MysqlInit();
    blog_system::TableTag tt(mysql_fd);
    Json::Value tmp;
    //tmp["name"]="lingfeihe";
    //tt.Insert(tmp);
    //tmp.clear();
    //tmp["name"]="nihao";
    // tt.Insert(tmp);

    tt.Delete(1);
    tt.Delete(5);
    // tt.GetAll(&tmp);
    tmp.clear();
    tmp["name"]="zhangsan";
    tt.Update(3,tmp);
    Json::Reader reader ; //JSON反序列化
    // Json::Writer writer; //json序列化，但是这是一个基类，不能直接使用
    //  Json::StyledWriter writer; //一般使用这个
    //cout<<writer.write(tmp)<<endl;
    blog_system::MysqlRelsase(mysql_fd); 
}
//定义全局变量,三个指针类变量
blog_system::TableUser* table_user; 
blog_system::TableTag*  table_tag;
blog_system::TableBlog* table_blog;

//指定的回调函数
void InsertUser(const httplib::Request& req,httplib::Response& rsp)
{
    //对于添加用户，需要在req的正文中得到用户名（为json字符串类型），再将其解析为为json::Value对象，再传入数据库操作函数，并把把结果填充在rsp响应中
    std::string json_body=req.body;
    Json::Reader reader;
    Json::Value root; //保存解析的参数
    bool ret = reader.parse(json_body,root);
    if(ret ==false)
    {
        printf("插入用户时，字符串解析为Json::Value有误\n");
        rsp.status = 400; //请求失败
        return ;
    }
    
    //接着调用table_user中的Insert接口插入数据
    ret = table_user->Insert(root);
    if(ret == false)
    {
        printf("插入用户时，数据库插入错误\n");
        rsp.status=500;//服务器内部错误
        return ;
    }
    //填充rsp响应信息
    rsp.status=200;//也可以不写，因为响应状态码默认为200
    return ;
}
void DeleteUser(const httplib::Request& req,httplib::Response& rsp)
{
    //获取要删除用户的id /user/id  /user/(\d+) //这里的id信息存在资源路径中
    int user_id=stoi(req.matches[1]);//matches[0]存的是整个资源路径字符串
    //执行数据库操作
    bool ret = table_user->Delete(user_id);
    if(ret ==false)
    {
        printf("删除用户时，数据库操作失败\n");
        rsp.status=500;
        return ;
    }
    return ;
}
void UpdateUser(const httplib::Request& req,httplib::Response& rsp)
{
    //需要获取id和正文要修改的内容
    int user_id=stoi(req.matches[1]);
    Json::Reader reader;
    Json::Value root;
    bool ret = reader.parse(req.body,root);
    if(ret ==false)
    {
        printf("修改用户名时，参数解析失败\n");
        rsp.status=400;
        return ;
    }
    ret = table_user->Update(user_id,root);
    if(ret == false)
    {
        printf("修改用户信息时，数据库操作失败\n");
        rsp.status=500;
        return ;
    }
    return ;
}
void GetAllUser(const httplib::Request& req,httplib::Response& rsp)
{
    //获取数据库操作得到的所有信息的json::value对象，再将其序列化为json字符串类型保存在rsp正文中。
    Json::Value root;
    bool ret = table_user->GetAll(&root);
    if( ret ==false )
    {
        printf("获取用户信息时，数据库操作失败\n");
        rsp.status=500;
        return ;
    }
    //将其转化为json字符串，作为响应的正文，填充到rsp
    Json::FastWriter writer;
    string body;
    body=writer.write(root);
    //填充到rsp正文中 rsp.body=body;
    rsp.set_content(body.c_str() ,body.size(),"application/json");
    return ;
}
void GetOneUser (const httplib::Request& req,httplib::Response& rsp)
{
    //获取指定id的用户名
    int user_id = stoi(req.matches[1]);
    Json::Value root;
    bool ret =table_user->GetOne(user_id,&root);
    if(ret ==false)
    {
        printf("获取指定用户信息时，数据库操作失败\n");
        rsp.status=500;
        return ;
    }
    string body;
    Json::FastWriter writer;
    body=writer.write(root);
    rsp.set_content(body.c_str(),body.size(),"application/json");
    return ;
}
void InsertTag(const httplib::Request& req,httplib::Response& rsp)
{
    //从req中的正文提取要插入时tag名，再解析为json::value对象，再调用对应的数据库函数
    Json::Reader reader;
    Json::Value root;
    bool ret = reader.parse(req.body,root);
    if(ret ==false)
    {
        printf("添加标签时，数据解析失败\n");
        rsp.status=400;
        return ;
    }
    ret=table_tag->Insert(root);
    if(ret ==false)
    {
        printf("添加标签时，数据库操作失败\n");
        rsp.status=500;
        return ;
    }
    return ;
}
void DeleteTag(const httplib::Request& req,httplib::Response& rsp)
{
    //根据资源路径中的id 删除对应的标签
    int tag_id = stoi(req.matches[1]);
    bool ret = table_tag->Delete(tag_id);
    if(ret ==false)
    {
        printf("删除标签时，数据库操作失误\n");
        rsp.status=500;
        return ;
    }
    return ;
}
void UpdateTag(const httplib::Request& req,httplib::Response& rsp)
{
    //根据id和正文中的信息 更新标签表
    int tag_id=stoi(req.matches[1]);
    Json::Reader reader;
    Json::Value root;
    bool ret = reader.parse(req.body,root);
    if(ret ==false)
    {
        printf("更改标签信息时，数据解析失败\n");
        rsp.status =400;
        return ;
    }
    ret =table_tag->Update(tag_id,root);
    if(ret ==false)
    {
        printf("更新标签信息时，数据库操作失败\n");
        rsp.status=500;
        return ;
    }
    return ;
}
void GetAllTag(const httplib::Request& req,httplib::Response& rsp)
{
    //将获取到的json::Value 对象 序列化为json字符串，再填充到rsp的正文中
    Json:: Value root;
    bool ret = table_tag->GetAll(&root);
    if(ret ==false)
    {
        printf("查询所有标签时，数据库操作失误\n");
        rsp.status=500;
        return ;
    }
    string body;
    Json::FastWriter writer;
    body = writer.write(root);
    rsp.set_content(body.c_str(),body.size(),"application/json");
    return ;
}
void GetOneTag (const httplib::Request& req,httplib::Response& rsp)
{
    //获取指定id的标签信息
    //先提取出tag_id,再将查询的json::Value 对象序列化为json字符串，再填充rsp
    int tag_id= stoi(req.matches[1]);
    Json::Value root;
    bool ret= table_tag->GetOne(tag_id,&root);
    if(ret ==false)
    {
        printf("查询指定标签信息时，数据库操作失误\n");
        rsp.status=500;
        return ;
    }
    string body ;
    Json::FastWriter writer;
    body = writer.write(root);
    rsp.set_content(body.c_str(),body.size(),"application/json");
    return ;
}

void InsertBlog(const httplib::Request& req,httplib::Response& rsp)
{
    //将正文中的json字符串解析为json::Value 对象 ，再调用数据库操作完成插入
    Json::Value root;
    Json::Reader reader;
    bool ret = reader.parse(req.body,root);
    if(ret ==false)
    {
        printf("增加博客时，数据解析失败\n");
        rsp.status=400;
        return ;
    }
    ret=table_blog->Insert(root);
    if(ret ==false)
    {
        printf("增加博客时，数据库操作失败\n");
        rsp.status=500;
        return ;
    }
    return ;
}
void DeleteBlog(const httplib::Request& req,httplib::Response& rsp)
{
    //根据资源路径中的id删除指定的博客
    int blog_id=stoi(req.matches[1]);
    bool ret= table_blog->Delete(blog_id);
    if(ret ==false)
    {
        printf("删除博客时，数据库操作失误\n");
        rsp.status=500;
        return ;
    }
    return ;
}
void UpdateBlog(const httplib::Request& req,httplib::Response& rsp)
{
    //根据id修改指定的博客信息
    int blog_id =stoi(req.matches[1]);
    Json::Reader reader;
    Json::Value root;
    bool ret = reader.parse(req.body,root);
    if(ret ==false)
    {
        printf("更新博客信息时，数据解析错误\n");
        rsp.status=400;
        return ;
    }
    ret= table_blog->Update(blog_id,root);
    if(ret ==false)
    {
        printf("更新博客信息时，数据库操作失败\n");
        rsp.status=500;
        return ;
    }
    return ;
}
void GetAllBlog(const httplib::Request& req,httplib::Response& rsp)
{
    //获取所有的博客又分为三种情况
    //获取所有、获取指定标签的所有博客/blog?tag_id=id、获取指定用户的所有博客/blog?user_id=id  可以根据 请求里的字符串参数来判断
    Json::Value root;
    if(req.has_param("tag_id")) //查询请求中是否有字符串"tag_id"
    {
        int tag_id=stoi(req.get_param_value("tag_id"));
        bool ret = table_blog->GetTagAll(tag_id,&root);
        if(ret ==false)
        {
            printf("获取此标签的所有博客失败\n");
            rsp.status=500;
            return ;
        }
    }
    else if(req.has_param("user_id"))
    {
        int user_id=stoi(req.get_param_value("user_id"));
        bool ret= table_blog->GetUserAll(user_id,&root);
        if(ret ==false)
        {
            printf("获取此用户的所有博客失败\n");
            rsp.status=500;
            return ;
        }
    }
    else //否则，就是获取所有的
    {
        bool ret=table_blog->GetAll(&root);
        if(ret ==false)
        {
            printf("获取所有博客失败\n");
            rsp.status = 500;
            return ;
        }
    }
    //再将其填充到rsp的正文中
    Json::FastWriter writer;
    string body;
    body=writer.write(root);
    rsp.set_content(body.c_str(),body.size(),"application/json");
    return ;
}
void GetOneBlog (const httplib::Request& req,httplib::Response& rsp)
{   
    //获取指定id的博客
    int blog_id=stoi(req.matches[1]);
    Json::Value root;
    bool ret =table_blog->GetThisOne(blog_id,&root);
    if(ret ==false)
    {
        printf("获取当前博客失败\n");
        rsp.status=500;
        return ;
    }
    string body ;
    Json::FastWriter writer;
    body = writer.write(root);
    rsp.set_content(body.c_str(),body.size(),"application/json");
    return ;
}    

int main()
{
    MYSQL* mysql=blog_system::MysqlInit();
    if(mysql == NULL )
    {
        return -1;
    }
    //初始化三个全局变量
    table_user=new blog_system::TableUser(mysql); 
    table_tag=new blog_system::TableTag(mysql); 
    table_blog=new blog_system::TableBlog(mysql);

    httplib::Server server;
    //httplib::Server::set_base_dir()//设置url中资源路径的相对根目录
    server.set_base_dir(WWWROOT);
    //注册路由函数，realful风格  ，为map键值对格式，请求方法和url中的请求资源路径，对应相应的处理函数
    
    //利用正则表达式中的R"()",来去除特殊的转义字符语义
    //增删改查
    //根据请求方法和资源路径 来调用不同的处理函数
    server.Post(R"(/user)",InsertUser);
    server.Delete(R"(/user/(\d+))",DeleteUser);
    server.Put(R"(/user/(\d+))",UpdateUser);
    server.Get(R"(/user)",GetAllUser);
    server.Get(R"(/user/(\d+))",GetOneUser);
    
    server.Post(R"(/tag)",InsertTag);
    server.Delete(R"(/tag/(\d+))",DeleteTag);
    server.Put(R"(/tag/(\d+))",UpdateTag);
    server.Get(R"(/tag)",GetAllTag);
    server.Get(R"(/tag/(\d+))",GetOneTag);
    
    server.Post(R"(/blog)",InsertBlog);
    server.Delete(R"(/blog/(\d+))",DeleteBlog);
    server.Put(R"(/blog/(\d+))",UpdateBlog);
    server.Get(R"(/blog)",GetAllBlog); //根据参数选择 所有（blog） 还是指定标签的所有(blog?tag_id=x) 还是指定用户的所有(blog?user_id=x)
    server.Get(R"(/blog/(\d+))",GetOneBlog);
    server.listen("0.0.0.0",9000);
    blog_system::MysqlRelsase(mysql);//释放资源
    return 0;
}
