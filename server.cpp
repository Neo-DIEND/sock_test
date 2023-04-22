/*
 * 程序名：server.cpp，此程序用于演示socket通信的服务端
 * 作者：C语言技术网(www.freecplus.net) 日期：20190525
*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <vector>
using namespace std;
 
void* fun(void*arg);
void cleanfun(void*arg);


int main(int argc,char *argv[])
{
  if (argc!=2)
  {
    printf("Using:./server port\nExample:./server 5005\n\n"); return -1;
  }

  // 第1步：创建服务端的socket。
  int listenfd;
  if ( (listenfd = socket(AF_INET,SOCK_STREAM,0))==-1) { perror("socket"); return -1; }
  
  // 第2步：把服务端用于通信的地址和端口绑定到socket上。
  struct sockaddr_in servaddr;    // 服务端地址信息的数据结构。
  memset(&servaddr,0,sizeof(servaddr));
  servaddr.sin_family = AF_INET;  // 协议族，在socket编程中只能是AF_INET。
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);          // 任意ip地址。
  //servaddr.sin_addr.s_addr = inet_addr("192.168.190.134"); // 指定ip地址。
  servaddr.sin_port = htons(atoi(argv[1]));  // 指定通信端口。
  if (bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0 )
  { perror("bind"); close(listenfd); return -1; }
 
  // 第3步：把socket设置为监听模式。//第二个参数为最大连接数
  if (listen(listenfd,5) != 0 ) { perror("listen"); close(listenfd); return -1; }
 
  // 第4步：接受客户端的连接。
  int  clientfd;                  // 客户端的socket。
  int  socklen=sizeof(struct sockaddr_in); // struct sockaddr_in的大小
  struct sockaddr_in clientaddr;  // 客户端的地址信息。

  pthread_t pid;
  vector<unsigned long >pid_vec;

  // pthread_attr_t attr;               //回收线程资源方法一
  // pthread_attr_init(&attr);
  // pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

  int ii = 1;

  while(true){
      clientfd=accept(listenfd,(struct sockaddr *)&clientaddr,(socklen_t*)&socklen);
      printf("客户端（%s）已连接。\n",inet_ntoa(clientaddr.sin_addr));
    
      if(pthread_create(&pid,NULL,fun,(void*)(long)clientfd) != 0){
        printf("***************线程创建失败***************\n");
        return -1;
      }
     // pthread_join(pid,NULL);
      pid_vec.push_back(pid);
      usleep(1000);
      
      printf("***************等待子线程退出***************\n");
     // pthread_detach(pid);             //回收线程资源方法二,,方法四为pthread_join,基本不用
      
      
      // sleep(3);
      // pthread_cancel(pid);//取消线程
      
     

      // 第5步：与客户端通信，接收客户端发过来的报文后，回复ok。
      // close(listenfd); close(clientfd); 
      //第四种回收线程资源方法，个方法不混合使用、join函数会阻塞程序

      printf("***************子线程已经退出***************\n");
  }
}
void *fun(void *arg){

  pthread_detach(pthread_self());      //回收线程资源方法三，线程分离后，系统自动回收资源
   pthread_cleanup_push(cleanfun,NULL);
  // pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,NULL);//设置为PTHREAD_CANCEL_DISABLE，取消函数无效，
  // pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);//默认为PTHREAD_CANCEL_ENABLE

  //pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);//设置取消类型，立即取消
  //pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED,NULL);//设置取消类型，延迟取消,延迟到取消点，一般不用

  int  clientfd = (int)(long)arg;
  char buffer[1024];
  while (1)
  {
    int iret;
    memset(buffer,0,sizeof(buffer));
    if ( (iret=recv(clientfd,buffer,sizeof(buffer),0))<=0) // 接收客户端的请求报文。
    {
       printf("iret=%d\n",iret); break;   
    }
    printf("接收：%s\n",buffer);
 
    strcpy(buffer,"ok");
    if ( (iret=send(clientfd,buffer,strlen(buffer),0))<=0) // 向客户端发送响应结果。
    { perror("send"); break; }
    printf("发送：%s\n",buffer);
 
  }
  
  pthread_cleanup_pop(1);
  //printf("\nafter pthread_cleanup_pop(1)\n");
  // 第6步：关闭socket，释放资源。
  //pthread_cleanup_pop(1);
  return NULL;
  
}

void cleanfun(void *arg){
  printf("**********cleanfun()**********");
}