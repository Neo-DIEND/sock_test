#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

void * MyAccept(void*clientfd);

int main(int argc,char *argv[]) {

    signal(SIGCHLD,SIG_IGN);//忽略子进程回收资源的信号，子进程变成孤儿进程，由系统自动回收

    if(argc != 2){
        printf("Using:./MyServer port\nExample:./server 5005\n\n"); return -1;}

    int listenfd;
    if((listenfd = socket(AF_INET,SOCK_STREAM,0)) ==  -1){
        printf("socket error\n");return -1;}

    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family  = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));

    if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) != 0){
        printf("bind error\n");
        return -1;
    }

    if(listen(listenfd,5) != 0){
        printf("listen error\n");
        return -1;
    }

    int clientfd;
    socklen_t client_len;
    struct sockaddr_in client_addr;
    

    while(true){

        clientfd = accept(listenfd,(struct sockaddr*)&client_addr,&client_len);
        
        pthread_t pid;
        printf("客户端 [ %s ] 使用socket [ %d ] 已连接\n",
        inet_ntoa(client_addr.sin_addr),clientfd);

        if(pthread_create(&pid,NULL,MyAccept,(int*)(long)clientfd) < 0){
            printf("pthread_create error\n");
        }
        
    }
    

    close(listenfd);



    return 0;
}

void * MyAccept(void*clientfd){

    pthread_detach(pthread_self());
    printf("\t************线程分离************\t\n");
    int iter;char buff[1024];

    while(true){
        if((iter = recv((int)(long)clientfd,buff,sizeof(buff),0)) <= 0){
            printf("recv : %d\n",iter);
            break;
        }
        printf("socket : [ %d ] 接收 : %s \n",(int)(long)clientfd,buff);
        strcpy(buff,"ok");
        if((send((int)(long)clientfd,buff,sizeof(buff),0)) <= 0){
            printf("send error\n");
            return NULL;
        }
    }
    close((int)(long)clientfd);
    printf("\n*******close((int)(long)clientfd)*******\n");
    return NULL;
   
}