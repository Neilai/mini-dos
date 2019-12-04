#include "utility.h"
#include <cstdlib>
using namespace std;
int main(int argc, char *argv[])
{
    struct sockaddr_in serverAddr, workerAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    char message[BUF_SIZE];

    // 创建socket
    dos::Operation operation;
    operation.set_operation(dos::Operation::QUERY);
    operation.SerializeToArray(message,BUF_SIZE);
    dos::Operation deserializedOperation;
    deserializedOperation.ParseFromArray(message, BUF_SIZE);
    cout << "deserializedOperation debugString:" << deserializedOperation.DebugString();
    int sockMaster = socket(PF_INET, SOCK_STREAM, 0);
    if (connect(sockMaster, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("connect error");
        exit(-1);
    }
    if (send(sockMaster, message, strlen(message), 0) < 0)
    {
        printf("send msg erroro");
        return 0;
    }

    workerAddr.sin_family = PF_INET;
    workerAddr.sin_port = htons(WORKER_PORT);
    workerAddr.sin_addr.s_addr = inet_addr(WORKER_IP);
    const char *computing = "client请求计算\n";
    int sockWorker = socket(PF_INET, SOCK_STREAM, 0);
    if (connect(sockWorker, (struct sockaddr *)&workerAddr, sizeof(workerAddr)) < 0)
    {
        perror("connect error");
        exit(-1);
    }
    if (send(sockWorker, computing, strlen(computing), 0) < 0)
    {
        printf("send msg erroro");
        return 0;
    }
}

