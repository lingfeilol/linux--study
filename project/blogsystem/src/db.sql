create database if not exists blog_system;
use blog_system;

create table if not exists table_tag(
    id int primary key auto_increment comment '标签ID',
    name varchar(30)  unique comment '标签名称'
  );

create table if not exists table_user(
      id int primary key auto_increment comment '用户ID',
      name varchar(30) unique comment '用户名'
    );
create table if not exists table_blog(
     id int primary key auto_increment comment '博客ID',
     tag_id int comment '博客所属标签ID',
     user_id int comment '博客所属用户ID',
     title varchar(255) comment '博客标题',
     content text comment '博客内容',
     ctime datetime comment '博客更改时间',
      foreign key (tag_id) references table_tag(id),
     foreign key (user_id) references table_user(id)
  );
