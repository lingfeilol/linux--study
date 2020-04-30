#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

volatile  int counter=20;
pthread_rwlock_t rwlock;

/* 2个线程不定时写同一全局资源，3个线程不定时读同一全局资源 */
void *th_write(void *arg)
{
  int* i=(int*)arg;
  while (1) 
  {
    pthread_rwlock_wrlock(&rwlock);
    if(counter<=0)
    {
      pthread_rwlock_unlock(&rwlock);
      break;//终止
    }
    printf("=======writeer  %lu: writeed_counter=%d\n", pthread_self(), --counter);
    pthread_rwlock_unlock(&rwlock);
    sleep(1);

  }
  return NULL;

}


void *th_read(void *arg)
{
  int* i = (int*)arg;

  while (1)
  {
    pthread_rwlock_rdlock(&rwlock);
    if(counter<=0)
    {
      pthread_rwlock_unlock(&rwlock);
      break;//终止
    }
    
    printf("-------reader %lu: readed_counter: %d\n", pthread_self(), counter);
    pthread_rwlock_unlock(&rwlock);
    sleep(1);

  }
  return NULL;

}

int main(void)
{
  int i;
  pthread_t tid[5];
  pthread_rwlock_init(&rwlock, NULL);

  for (i = 0; i < 2;i++)
    pthread_create(&tid[i], NULL, th_write,&i);
  for (i = 2; i < 5; i++)
    pthread_create(&tid[i], NULL, th_read,&i);
  for (i = 0; i < 5; i++)
    pthread_join(tid[i], NULL);

  pthread_rwlock_destroy(&rwlock);
  return 0;

} 
