#include "utility.h"
#include<map>
#include<tuple>
using namespace std;
int main(int argc, char *argv[])
{
  map<int,tuple<char*,int,int>> socketMap;
  struct sockaddr_in serverAddr;
  serverAddr.sin_family = PF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
  int listener = socket(PF_INET, SOCK_STREAM, 0);
  if (bind(listener, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
  {
    perror("bind error");
    exit(-1);
  }
  //监听
  int ret = listen(listener, 10);
  if (ret < 0)
  {
    perror("listen error");
    exit(-1);
  }
  printf("Start to listen: %s\n", SERVER_IP);
  if (ret < 0)
  {
    perror("listen error");
    exit(-1);
  }
  printf("Start to listen: %s\n", SERVER_IP);
  //在内核中创建事件表
  int epfd = epoll_create(EPOLL_SIZE);
  if (epfd < 0)
  {
    perror("epfd error");
    exit(-1);
  }
  printf("epoll created, epollfd = %d\n", epfd);
  static struct epoll_event events[EPOLL_SIZE];
  //往内核事件表里添加事件
  addfd(epfd, listener, true);
  char message[BUF_SIZE];
  while (1)
  {
    //epoll_events_count表示就绪事件的数目
    int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
    if (epoll_events_count < 0)
    {
      perror("epoll failure");
      break;
    }

    printf("epoll_events_count = %d\n,map size:%d \n", epoll_events_count,socketMap.size());
    //处理这epoll_events_count个就绪事件
    for (int i = 0; i < epoll_events_count; ++i)
    {
      int sockfd = events[i].data.fd;
      printf("events :%d \n",events[i].events);
      //新用户连接
      if (sockfd == listener)
      {
        struct sockaddr_in client_address;
        socklen_t client_addrLength = sizeof(struct sockaddr_in);
        int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);
        tuple<char*,int,int> worker(inet_ntoa(client_address.sin_addr),ntohs(client_address.sin_port),0);
      
        printf("client connection from: %s : % d(IP : port), clientfd = %d \n",
              get<0>(worker),
              get<1>(worker),
              clientfd);
        socketMap.insert(pair<int,tuple<char*,int,int>>(clientfd,worker));
        addfd(epfd, clientfd, true);
      }
      else if (events[i].events & (EPOLLRDHUP|EPOLLHUP | EPOLLERR))
      {
        printf("closing fd %d", events[i].data.fd);
        if (close(events[i].data.fd) == -1)
          perror("close failure");
      }
      else if (events[i].events & EPOLLIN)
      {
        int ret = recv(events[i].data.fd, message, BUF_SIZE, 0);
        //printf("ret : %d \n",ret!=0);
        if(ret!=0){
          printf("message :%s\n", message);
          continue;
        }
        map<int,tuple<char*,int,int>>::iterator it;
        it=socketMap.find(events[i].data.fd);
        if(it==socketMap.end())
          printf("没有找到这个描述符");
        else{
          socketMap.erase(it);
          printf("close !!!");
        }
      }
    }
  }
  return 0;
}
