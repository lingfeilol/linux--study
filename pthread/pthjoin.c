#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
void *thread_run( void * arg  )
{
    pthread_detach(pthread_self());//将默认可结合性改为分离属性
      printf("%s\n", (char*)arg);
        return NULL;

}
int main( void  )
{
    pthread_t tid;
    if ( pthread_create(&tid, NULL, thread_run,"change arr") != 0  ) {
          printf("create thread error\n");
              return 1;
                  
    }
      int ret = 0;
        sleep(1);//很重要，要让线程先分离，再等待
          if ( pthread_join(tid, NULL ) == 0  ) { //分离属性设置失败就会调用此函数来回收资源
              printf("pthread change detach failed\n");
              ret = 0;
            }
          else{
                printf("pthread change detach succesed\n");
                    ret = 1;
                      
          }
            return ret;

}
