#include "utility.h"
#include<cstdlib>
int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddr,workerAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 创建socket
    const char *message = "worker注册\n";
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("connect error");
        exit(-1);
    }
    if (send(sock, message, strlen(message), 0) < 0)
    {
        printf("send msg erroro");
        return 0;
    }

    workerAddr.sin_family = PF_INET;
    workerAddr.sin_port = htons(atoi(argv[1]));
    workerAddr.sin_addr.s_addr = inet_addr(WORKER_IP);
    int listener = socket(PF_INET, SOCK_STREAM, 0);
    if (bind(listener, (struct sockaddr *)&workerAddr, sizeof(workerAddr)) < 0)
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
    printf("Start to listen: %s\n", WORKER_IP);
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
    char request[BUF_SIZE];
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
            //新用户连接
            if (sockfd == listener)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);
                printf("client connection from: %s : % d(IP : port), clientfd = %d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), clientfd);
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
                int ret = recv(events[i].data.fd, request, BUF_SIZE, 0);
                //printf("ret : %d \n",ret!=0);
                if (ret != 0)
                {
                    printf(" 进行计算:%s\n", request);
                    continue;
                }
            }
        }
    }
}

