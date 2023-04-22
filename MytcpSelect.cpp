#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>


int initserver(int port);

int main(int argc,char *argv[]) {

    if(argc != 2){
       printf("usage: ./tcpselect port\n"); return -1;
    }
    int listensock = initserver(atoi(argv[1]));

    if(listensock < 0){
        printf("listensock faild\n");
        return -1;
    }

    fd_set readfdset;
    int maxfd;

    FD_ZERO(&readfdset);
    FD_SET(listensock,&readfdset);
    maxfd = listensock;

    while(true){
        fd_set tempfdset = readfdset;

        int nfds = select(maxfd+1,&tempfdset,NULL,NULL,NULL);//这里要用临时集合

        if(nfds == -1){
            printf("select faild \n");
            return -1;
        }
        if(nfds == 0){
            printf("select timeout \n");
            return -1;
        }

        for(int eventfd = 0 ; eventfd <= maxfd; eventfd++){
           
            if((FD_ISSET(eventfd,&tempfdset)) <= 0)continue;

            if(eventfd == listensock){
                struct sockaddr_in client;
                socklen_t len = sizeof(client);
                int clientsock = accept(eventfd,(struct sockaddr*)&client,&len);
                if(clientsock < 0){
                    printf("accept faild \n");
                    return -1;
                }
                FD_SET(clientsock,&readfdset);

                if(maxfd < clientsock)maxfd = clientsock;
                printf("listensock : [ %d ] \t\t clientsock : [ %d ]\n",listensock,clientsock);

                continue;
                
            }
            else{
                char buffer[1024];
                memset(buffer,0,sizeof(buffer));

                ssize_t readlen = read(eventfd,buffer,sizeof(buffer));
                if(readlen <=0 ){
                    printf("read faild \n");

                    close(eventfd);
                    FD_CLR(eventfd,&readfdset);
                    

                    if(eventfd == maxfd){
                        for(int ii = maxfd ; ii > 0 ; ii--){
                            if(FD_ISSET(ii,&readfdset)){
                                maxfd = ii;
                                break;
                            }
                        }
                        printf("maxfd=%d\n",maxfd);
                    }
                    continue;
                }
                printf("接收: socket[ %d ] : %s\n",eventfd,buffer);
                write(eventfd,buffer,sizeof(buffer));
            }
        }
    }

    return 0;
}



int initserver(int port){

    int sock = socket(AF_INET,SOCK_STREAM,0);

    if(sock < 0){
        printf("socket faild \n");
        return -1;
    }
    int opt = 1;unsigned int len = sizeof(opt);
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,len);
    setsockopt(sock,SOL_SOCKET,SO_KEEPALIVE,&opt,len);

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port =htons(port);


    if((bind(sock,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
        printf("bind faile \n");
        return -1;
    }
    if((listen(sock ,5)) != 0){
        printf("listen faild\n");
        return -1;
    }


    return sock;

}