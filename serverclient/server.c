#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    umask(0);
      if(mkfifo("mypipe", 0644) < 0){  //创建命名管道文件
          perror("mkfifo");
        }
        int rfd = open("mypipe", O_RDONLY);  //打开管道文件 文件描述符为rfd
        if(rfd < 0){
              perror("open");
                
        }
          char buf[1024];
          while(1){
                buf[0] = 0;
                    printf("Please wait...\n");
                        ssize_t s = read(rfd, buf, sizeof(buf)-1);//从管道文件里读数据
                        if(s > 0 ){
                                buf[s-1] = 0;
                                      printf("client say# %s\n", buf); 
                                          
                        }else if(s == 0){
                                printf("client quit, exit now!\n");
                                      exit(EXIT_SUCCESS);
                                          
                        }else{
                                perror("read");
                                    
                        }
                          
          }
            close(rfd);
              return 0;

}
