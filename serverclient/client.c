#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main()
{
    int wfd = open("mypipe", O_WRONLY);//打开管道文件
    if(wfd < 0){
          perror("open");
            
    }
      char buf[1024];
      while(1){
            buf[0] = 0;
                printf("Please Enter# ");
                    fflush(stdout);  //刷新缓冲区
                        ssize_t s = read(0, buf, sizeof(buf)-1); //从标准输入（键盘）里写到buf中
                        if(s > 0 ){
                                buf[s] = 0;
                                      write(wfd, buf, strlen(buf)); //再写入管道文件
                                          
                        }else if(s <= 0){
                                perror("read");
                                    
                        }
                          
      }
        close(wfd);
          return 0;

}
