#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include<unistd.h>
int main( void  )
{
    pid_t pid;
    if ((pid=fork()) == -1){
          perror("fork");
              exit(1);
                  
    }
    if (pid == 0  ){
         sleep(20);
         exit(10); //子进程退出信息返回给父进程，通过位图&运算，得出退出码
                
    } 
    else {
          int st;
          int ret = wait(&st); // 接收 退出信息状态，不是退出码，退出码要用位图运算的
          if ( ret > 0 && ( st & 0X7F  ) == 0  ){ // 说明第低七位全为0，正常退出
                 printf("child exit code:%d\n", (st>>8)&0XFF); //&运算，输出退出码
          } 
           else if( ret > 0  ) {   //(st & 0X7F ) > 0 说明第七位里有值，异常退出
                printf("sig code : %d\n", st&0X7F ); //&运算 输出退出码
         }
                
    }

}
