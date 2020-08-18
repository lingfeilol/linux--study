#include<string.h>

//清除字符串右边的空格
void Rspace(char* str)
{
    if(str ==NULL)
        return ;
    int len=strlen(str);
    while(len > 0&& str[len-1]==' ')
        str[--len]=0;
}
//清除左边
void Lspace(char* str)
{
    if(str ==NULL)
        return ;
    //双指针，且需要移位拷贝
    char* ptmp=str;
    if(*ptmp != ' ' )
        return ;
    while(*ptmp != '\0')
    {
        if(*ptmp != ' ')
            break;
        ptmp++;//走到不是空格的地方
    }
    //拷贝
    char* ptmp2=str;
    while(*ptmp !='\0')
    {
        *ptmp2=*ptmp;
        ptmp++;
        ptmp2++;
    }
    *ptmp2='\0';
}
