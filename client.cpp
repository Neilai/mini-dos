#include "utility.h"
#include <cstdlib>
using namespace std;
int main(int argc, char *argv[])
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    struct sockaddr_in serverAddr, workerAddr;
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    char message[BUF_SIZE];

    // 创建socket
    dos::Operation operation;
    dos::Operation deserializedOperation;
    operation.set_operation(dos::Operation::QUERY);
    operation.SerializeToArray(message, BUF_SIZE);
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
    int ret = recv(sockMaster, message, BUF_SIZE, 0);
    



    //deserializedOperation.ParseFromArray(message, BUF_SIZE);
    //cout << "deserializedOperation debugString:" << deserializedOperation.DebugString();
    //const dos::Operation::DistributeTask& task_content = deserializedOperation.task(0);
    //cout<<"size:"<<deserializedOperation.result_size()<<endl;
    //cout<<"DistributeTask:"<<task_content.operation_num_type()<<endl;
    //cout<<task_content.operation_num_one()<<task_content.operation_label()<<task_content.operation_num_two()<<endl;

    deserializedOperation.ParseFromArray(message, BUF_SIZE);

    workerAddr.sin_family = PF_INET;
    workerAddr.sin_port = htons(deserializedOperation.port());
    workerAddr.sin_addr.s_addr = inet_addr(deserializedOperation.ip().c_str());

    operation.set_operation(dos::Operation::COMPUTE);
    dos::Operation::DistributeTask *task = operation.add_task();
    task->set_operation_num_type("int");
    task->set_operation_num_one("1");
    task->set_operation_num_two("9999");
    task->set_operation_label("+");
    operation.SerializeToArray(message, BUF_SIZE);

    int sockWorker = socket(PF_INET, SOCK_STREAM, 0);
    if (connect(sockWorker, (struct sockaddr *)&workerAddr, sizeof(workerAddr)) < 0)
    {
        perror("connect error");
        exit(-1);
    }
    if (send(sockWorker,message, strlen(message), 0) < 0)
    {
        printf("send msg erroro");
        return 0;
    }
}

