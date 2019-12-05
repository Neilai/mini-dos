#include <iostream>
#include <list>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include<string>
#include<string.h>
#include "mini_dos.pb.h"
#include <netinet/tcp.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8888
#define WORKER_IP  "127.0.0.1"
#define WORKER_PORT 8889
#define EPOLL_SIZE 5000
#define BUF_SIZE 0xFFFF
using namespace std;
int setnonblocking(int sockfd)
{
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK);
    return 0;
}
void noNagle (int socket)
{
        int on = 1;
        if (setsockopt (socket, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on)) < 0)
            printf ("Can not disable nagle algorithm\n");
}
void addfd(int epollfd, int fd, bool enable_et)
{
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (enable_et)
        ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
    noNagle(fd);
    printf("新描述符添加到了epoll\n");
}
template<typename T>
T compute(T a, T b,string op)
{
    cout<<"进入计算函数"<<endl;
    //sleep(2);
    if(op[0]=='+')
      return a + b;
    else if(op[0]=='-')
      return a-b;
    else if(op[0]=='*')
      return a*b;
    else if(op[0]=='/')
      return a/b;
}
