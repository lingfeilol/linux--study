#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list.h>
//初始化链表
list_t *list_init()
{
    list_t *p =(list_t*)malloc(sizeof(list_t));
    memset(p,0x00,sizeof(list_t));
    p->head=NULL;
    p->len=0;
}

//插入数据
void list_insert(list_t* head,const char* key , const char* value)
{
    //节点开空间，并赋值
    node_t* newnode=(node_t*)malloc(sizeof(node_t));
    memset(newnode,0x00,sizeof(node_t));

    newnode->name=malloc(strlen(key)+1);
    strncpy(newnode->name,key,strlen(key));

    newnode->value=malloc(strlen(value)+1);
    strncpy(newnode->value,value,strlen(value));
    //开始头插
    if(head->head== NULL)
    {
        head->head=newnode;
    }
    else 
    {
        newnode->next=head->head;
        head->head=newnode;
    }
    head->len++;
}



