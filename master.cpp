#include "utility.h"
#include <map>
#include <tuple>
using namespace std;
int main(int argc, char *argv[])
{
  GOOGLE_PROTOBUF_VERIFY_VERSION;
  map<int, tuple<char *, int, int>> socketMap;
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
  while (1)
  {
    //epoll_events_count表示就绪事件的数目
    int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
    if (epoll_events_count < 0)
    {
      perror("epoll failure");
      break;
    }

    //处理这epoll_events_count个就绪事件
    for (int i = 0; i < epoll_events_count; ++i)
    {
      int sockfd = events[i].data.fd;
      printf("events :%d \n", events[i].events);
      char *message = new char[BUF_SIZE];
      //新用户连接
      if (sockfd == listener)
      {
        struct sockaddr_in client_address;
        socklen_t client_addrLength = sizeof(struct sockaddr_in);
        int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);
        printf("新连接: %s : % d(IP : port), clientfd = %d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), clientfd);

        addfd(epfd, clientfd, true);
      }
      else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
      {
        printf("closing fd %d", events[i].data.fd);
        if (close(events[i].data.fd) == -1)
          perror("close failure");
      }
      else if (events[i].events & EPOLLIN)
      {
        printf("描述符为%d \n", events[i].data.fd);
        int ret = recv(events[i].data.fd, message, BUF_SIZE, 0);
        ret = recv(events[i].data.fd, message, BUF_SIZE, 0);
        dos::Operation deserializedOperation;
        deserializedOperation.ParseFromArray(message, BUF_SIZE);
        cout << "deserializedOperation debugString:" << deserializedOperation.DebugString();

        printf("ret : %d \n", ret);
        if (ret != 0)
        {
          if (deserializedOperation.operation() == 1)
          {
            printf("master获取到了worker注册信息");
            char *ip = "127.0.0.1";
            tuple<char *, int, int> worker(ip, deserializedOperation.port(), 0);

            printf("worker 信息: %s : % d(IP : port)",
                   get<0>(worker),
                   get<1>(worker));
            socketMap.insert(pair<int, tuple<char *, int, int>>(int(events[i].data.fd), worker));
          }
          else if (deserializedOperation.operation() == 0)
          {
            printf("master获取到了client请求信息\n");
            int minId = socketMap.begin()->first;
            map<int, tuple<char *, int, int>>::iterator it = socketMap.begin();
            while (it != socketMap.end())
            {
              if (get<2>(it->second) < get<2>(socketMap[minId]))
                minId = it->first;
              it++;
            }
            printf("分配描述符%d ip:%s port:%d\n",minId, get<0>(socketMap[minId]),get<1>(socketMap[minId]));
          }
          continue;
        }
        map<int, tuple<char *, int, int>>::iterator it;
        it = socketMap.find(int(events[i].data.fd));
        if (it == socketMap.end())
          printf("关闭描述符，该描述符没有在map中注册\n");
        else
        {
          socketMap.erase(it);
          printf("关闭描述符，并从map中移除\n");
        }
      }
      delete[] message;
    }
    printf("epoll_events_count = %d\nmap size:%d \n", epoll_events_count, socketMap.size());
  }
  return 0;
}


