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
  //printf("master开始监听，监听地址: %s\n", SERVER_IP);
  if (ret < 0)
  {
    perror("listen error");
    exit(-1);
  }
  //printf("Start to listen: %s\n", SERVER_IP);
  //在内核中创建事件表
  int epfd = epoll_create(EPOLL_SIZE);
  if (epfd < 0)
  {
    perror("epfd error");
    exit(-1);
  }
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

    //处理这epoll_events_count个就绪事件
    for (int i = 0; i < epoll_events_count; ++i)
    {
      int sockfd = events[i].data.fd;
      memset(message, 0, BUF_SIZE);
      //char *message = new char[BUF_SIZE];
      //新用户连接
      if (sockfd == listener)
      {
        struct sockaddr_in client_address;
        socklen_t client_addrLength = sizeof(struct sockaddr_in);
        int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);
        printf("新连接:ip为 %s :端口为 % d,描述符为%d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), clientfd);

        addfd(epfd, clientfd, true);
      }
      else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
      {
        printf("关闭描述符 fd %d\n", events[i].data.fd);
        if (close(events[i].data.fd) == -1)
          perror("close failure");
      }
      else if (events[i].events & EPOLLIN)
      {
        printf("epoll处理的描述符为%d \n", events[i].data.fd);
        int ret = recv(events[i].data.fd, message, BUF_SIZE, 0);
        ret = recv(events[i].data.fd, message, BUF_SIZE, 0);
        dos::Operation deserializedOperation;
        deserializedOperation.ParseFromArray(message, BUF_SIZE);
        cout << "收包反序列化结果:" << endl
             << deserializedOperation.DebugString();

        printf("recv返回值: %d \n", ret);
        if (ret != 0)
        {
          if (deserializedOperation.operation() == 1)
          {
            printf("master获取到了worker注册信息");
            char *ip = "127.0.0.1";
            tuple<char *, int, int> worker(ip, deserializedOperation.port(), 0);

            printf("worker 信息: ip为%s : 端口为% d\n",
                   get<0>(worker),
                   get<1>(worker));
            socketMap.insert(pair<int, tuple<char *, int, int>>(int(events[i].data.fd), worker));
          }
          else if (deserializedOperation.operation() == 0)
          {
            printf("master获取到了client请求信息\n");
            dos::Operation operation;
            if(socketMap.size()==0){
              printf("没有worker可以分配\n");
              operation.set_ip(string(""));
              operation.set_port(0);
              operation.set_operation(dos::Operation::DISTRIBUTE);
              operation.SerializeToArray(message, BUF_SIZE);
              if (send(events[i].data.fd, message, strlen(message), 0) < 0)
                printf("send msg erroro\n");
              continue;
            }
            int minId = socketMap.begin()->first;
            map<int, tuple<char *, int, int>>::iterator it = socketMap.begin();
            while (it != socketMap.end())
            {
              if (get<2>(it->second) < get<2>(socketMap[minId]))
                minId = it->first;
              it++;
            }
            printf("为Client分配的worker信息:描述符为%d,ip为%s, port为%d\n", minId, get<0>(socketMap[minId]), get<1>(socketMap[minId]));
            operation.set_port(get<1>(socketMap[minId]));
            operation.set_ip(get<0>(socketMap[minId]));
            operation.set_operation(dos::Operation::DISTRIBUTE);
            operation.SerializeToArray(message, BUF_SIZE);
            if (send(events[i].data.fd, message, strlen(message), 0) < 0)
              printf("send msg erroro\n");
            char *ip;
            int port;
            int num;
            auto tp = make_tuple(ref(ip), ref(port), ref(num)) = socketMap[minId];
            num = num + 1;
            socketMap[minId] = tp;
            cout << "描述符" << minId << "当前负载为" << get<2>(socketMap[minId]) << endl;
          }
          else
          {
            char *ip;
            int port;
            int num;
            auto tp = make_tuple(ref(ip), ref(port), ref(num)) = socketMap[events[i].data.fd];
            num = num - 1;
            socketMap[events[i].data.fd] = tp;
            cout << "描述符" << events[i].data.fd << "当前负载为" << get<2>(socketMap[events[i].data.fd]) << endl;
            printf("收到worker完成任务通知\n");
          }
          continue;
        }
        map<int, tuple<char *, int, int>>::iterator it;
        it = socketMap.find(int(events[i].data.fd));
        if (close(events[i].data.fd) == -1)
          perror("close failure");
        if (it == socketMap.end())
          printf("关闭描述符，该描述符没有在map中注册\n");
        else
        {
          socketMap.erase(it);
          printf("关闭描述符，并从map中移除\n");
        }
      }
      //delete[] message;
    }
    printf("当前循环处理的epoll事件数= %d\n当前注册worker数:%d \n", epoll_events_count, socketMap.size());
  }
  return 0;
}
