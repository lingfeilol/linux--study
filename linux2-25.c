#include<stdio.h>
#include<unistd.h>

int main()
{
        int i=0;
        char out[102];
        const char* tmp="|/-\\";
        memset(out,0,sizeof(out));
        while(i<=100)
        {
                printf("[%c][%-100s][%d%%]\r",tmp[i%4],out,i);
                fflush(stdout);
                out[i++]='=';
                usleep(100000);
        }
        printf("\n");
        return 0;
}

