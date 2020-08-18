#include<list.h> 
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static list_t* head=NULL;
//加载，在程序启动时将所有的配置设置加载到内存
void load(const char* conf_name)
{
    if(conf_name == NULL)
        return ;
    FILE * fp = fopen(conf_name,"r");
    if(fp == NULL)
        return ;
    //一行一行读取
    char linebuf[501];
    head=list_init();
    while( !feof(fp) )
    {
        memset(linebuf,0x00,sizeof(linebuf));
        if(fgets(linebuf,500,fp) == NULL) //读一行
            continue;
        if(linebuf[0] == '\0') //空行
            continue;
        if(linebuf[0] == '#' || linebuf[0]==' '|| linebuf[0]=='\r' || linebuf[0]=='\n'
               || linebuf[0]=='\t')
            continue;
        //接下来就是有效的配置信息
lab:
        //去掉后面多余的一个空白字符
        if( linebuf[strlen(linebuf)-1] ==10 || 
                linebuf[strlen(linebuf)-1] ==13 ||
                linebuf[strlen(linebuf)-1] ==32)
        {
            linebuf[strlen(linebuf)-1] = '\0'; 
            goto lab;
        }

        if(linebuf[0]=='[')
            continue;
        //现在就是提取出来的kv形式的配置信息 key=value
        char *ptmp= strchr(linebuf,'=');
        if( ptmp != NULL )
        {
            //需要将在文件中的信息组织到内存中，可以使用容器来存储，链表/红黑树都可以 
            //在处理字符串两边有多余空格的问题
            char name[501]={};
            char value[501]={};
            strncpy(name,linebuf,(ptmp-linebuf));
            strcpy(value,ptmp+1);

            Lspace(name);
            Rspace(name);
            Lspace(value);
            Rspace(value);
            //保存到链表中
            list_insert(head,name,value);
        }
        
    }
    fclose(fp);
}
//在链表中找
//读取字符串的配置项，即key对应的value
const char* get_string(const char* key)
{
    node_t* tmp=head->head;
    while(tmp!= NULL)
    {
        if(strcmp(tmp->name,key)== 0)
            return tmp->value;
        tmp=tmp->next;
    }
    return NULL;
}
//读取整数类型的配置项
int get_int_default(const char* key,const int def)
{
    int  ret=def;
    node_t* tmp=head->head;
    while(tmp!= NULL)
    {
        if(strcmp(tmp->name,key)==0)
            ret=atoi(tmp->value);
            break;
        tmp=tmp->next;
    }
    return ret;

}
