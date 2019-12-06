#include "utility.h"
using namespace std;
int main(int argc, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    struct sockaddr_in serverAddr, workerAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    char message[BUF_SIZE];
    char resultBuffer[BUF_SIZE];
    char finishBuffer[BUF_SIZE];
    char requestBuffer[BUF_SIZE];

    // 创建socket
    dos::Operation operation;
    operation.set_port(atoi(argv[1]));
    operation.set_operation(dos::Operation::REGISTER);
    operation.SerializeToArray(message, BUF_SIZE);
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
    printf("worker开始监听: %s\n", WORKER_IP);
    int epfd = epoll_create(EPOLL_SIZE);
    if (epfd < 0)
    {
        perror("epfd error");
        exit(-1);
    }
    //printf("epoll created, epollfd = %d\n", epfd);
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
            memset(finishBuffer, 0, BUF_SIZE);
            memset(resultBuffer, 0, BUF_SIZE);
            memset(requestBuffer, 0, BUF_SIZE);
            int sockfd = events[i].data.fd;
            if (sockfd == listener)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrLength = sizeof(struct sockaddr_in);
                int clientfd = accept(listener, (struct sockaddr *)&client_address, &client_addrLength);
                printf("worker收到新连接: %s : % d(IP : port), clientfd = %d \n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port), clientfd);
                addfd(epfd, clientfd, true);
            }
            else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                printf("关闭描述符%d\n", events[i].data.fd);
                epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,NULL);
                if (close(events[i].data.fd) == -1)
                    perror("close failure");
            }
            else if (events[i].events & EPOLLIN)
            {
                int ret = recv(events[i].data.fd, requestBuffer, BUF_SIZE, 0);
                printf("ret : %d \n", ret);

                if (ret != 0)
                {
                    dos::Operation deserializedOperation;
                    deserializedOperation.ParseFromArray(requestBuffer, BUF_SIZE);
                    cout << "收包反序列化结果" << deserializedOperation.DebugString();
                    const dos::Operation::DistributeTask &task_content = deserializedOperation.task(0);
                    //cout<<"DistributeTask:"<<task_content.operation_num_type()<<endl;
                    //cout<<task_content.operation_num_one()<<task_content.operation_label()<<task_content.operation_num_two()<<endl;
                    //printf(" 进行计算...\n");
                    dos::Operation resultOperation;
                    resultOperation.set_operation(dos::Operation::RETURN);
                    dos::Operation::Result *result = resultOperation.add_result();
                    if (task_content.operation_num_type() == "int")
                    {
                        int computeResult = compute<int>(stoi(task_content.operation_num_one()), stoi(task_content.operation_num_two()), task_content.operation_label());
                        result->set_result_type("int");
                        result->set_result_value(to_string(computeResult));
                        cout << "计算结果:" << computeResult << endl;
                    }
                    if (task_content.operation_num_type() == "int64")
                    {
                        long long computeResult = compute<long long>(stoll(task_content.operation_num_one()), stoll(task_content.operation_num_two()), task_content.operation_label());
                        result->set_result_type("int64");
                        result->set_result_value(to_string(computeResult));
                        cout << "计算结果:" << computeResult << endl;
                    }
                    if (task_content.operation_num_type() == "float")
                    {
                        float computeResult = compute<float>(stof(task_content.operation_num_one()), stof(task_content.operation_num_two()), task_content.operation_label());
                        result->set_result_type("float");
                        result->set_result_value(to_string(computeResult));
                        cout << "计算结果:" << computeResult << endl;
                    }

                    resultOperation.SerializeToArray(resultBuffer, BUF_SIZE);
                    if (send(events[i].data.fd, resultBuffer, BUF_SIZE, 0) < 0)
                        printf("send msg erroro");
                    dos::Operation finishOperation;
                    dos::Operation deserializedFinish;
                    finishOperation.set_operation(dos::Operation::FINISH);
                    resultOperation.SerializeToArray(finishBuffer, BUF_SIZE);

                    deserializedFinish.ParseFromArray(finishBuffer, BUF_SIZE);
                    //cout << "发包反序列化结果" << deserializedFinish.DebugString();
                    if (send(sock, finishBuffer, BUF_SIZE, 0) < 0)
                        printf("send msg erroro");
                    continue;
                }
                epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                cout << "检测到关闭移除描述符" << endl;
                if (close(events[i].data.fd) == -1)
                    perror("close failure");
            }
        }
        printf("当前循环处理的epoll事件数= %d\n", epoll_events_count);
    }
}
