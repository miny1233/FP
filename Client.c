#include"lz4.h"
#include"lz4.c"
#include"threadpool.h"
#include"threadpool.c"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>


char listen[32];
char toserver[32];
int vs;
struct sockaddr_in servaddr;

int main(int argc,char** argv[]){
    memset(&listen,1,sizeof(listen));
    memset(&toserver,1,sizeof(toserver));
    for (size_t i = 0; i < argc; i++)
    {
        if(argv[i] == "-l"){
        strcpy(&listen,argv[i+1]);
        }
       else if (argv[i] == "-t")
        {
            strcpy(&toserver,argv[i+1]);
        }   
        else if(argv[i] == "-v"){
            vs = argv[i+1] - '0';
        }   
    }
    char *token;
    char toip;
    int toport;
    strcpy(&toip,strtok(toserver,":"));
    toport =atoi(strtok(toserver,":")); 
    char ip;
    int port;
    strcpy(&ip,strtok(listen,":"));
    port =atoi(strtok(listen,":")); 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr=inet_addr(&ip);
    servaddr.sin_port=htonl(port);
    int server;
    if(vs == 6){
       server = socket(AF_INET6, SOCK_STREAM, 0);
    }
    else
    {
        server = socket(AF_INET, SOCK_STREAM, 0);
    } 
    if(bind(server,(struct sockaddr*)&servaddr,sizeof(servaddr))==-1){
        perror("bind");
    }
    struct threadpool *pool = threadpool_init(100, 200);
    struct threadpool *dpool = threadpool_init(100, 200);
   return 0;
}

void * transport(void * args){
   struct connection *work =(struct connection*)args;
   char buff[1*1024*1024];
   char compress[1*1024*1024];
   int len;
   memset(&buff,1,sizeof(buff));
   memset(&compress,1,sizeof(buff));
   int fd;
   if (work->v == 6)
   {
        fd = socket(AF_INET6,SOCK_STREAM,0);
   }
   else{
        fd = socket(AF_INET,SOCK_STREAM,0);   
    }
    if(fd == 0 && connect(fd,work->addr,sizeof(work->addr)) != -1){
        perror("ERROR:Can't Create Socket");
        close(work->cli);
        free(work);
        free(buff);
        return -1;
    }
    while (1)
    {
       len=recv(work->cli,&buff,sizeof(buff),0);
       if(len == -1){
           perror("ERROR:recv");
           close(work->cli);
           close(fd);
        free(work);
        free(buff);
       }
       if (len == 0)
       {
           close(work->cli);
           close(fd);
        free(work);
        free(buff);
       }
       LZ4_compress_default(&buff,&compress,len,sizeof(compress));
      len = send(fd,&buff,sizeof(buff),0);
      if(len == -1){
           perror("ERROR:send");
           close(work->cli);
           close(fd);
        free(work);
        free(buff);
       }
    }
    
}



struct connection
{
    int cli;
    int v;
    struct sockaddr_in *addr;
    struct threadpool *dpool;
};
