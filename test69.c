#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
int main( void )
{
   pid_t pid;
   if ( (pid=fork()) == -1 )
       perror("fork"),exit(1);
   if ( pid == 0 ){
       sleep(20);

具体代码实现
进程的阻塞等待方式：
       exit(10);
   } else {
       int st;
       int ret = wait(&st);
        
       if ( ret > 0 && ( st & 0X7F ) == 0 ){ // 正常退出
           printf("child exit code:%d\n", (st>>8)&0XFF);
       } else if( ret > 0 ) { // 异常退出
           printf("sig code : %d\n", st&0X7F );
       }
   }
}