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
      int sockMaster = socket(PF_INET, SOCK_STREAM, 0);
      if (connect(sockMaster, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
      {
          perror("connect error");
          exit(-1);
      }
      while(1){
      // 创建socket
      dos::Operation operation;
      dos::Operation deserializedOperation;
      operation.set_operation(dos::Operation::QUERY);
      operation.SerializeToArray(message, BUF_SIZE);
      if (send(sockMaster, message, strlen(message), 0) < 0)
      {
          printf("send msg erroro");
          return 0;
      }
      int ret = recv(sockMaster, message, BUF_SIZE, 0);
      




      deserializedOperation.ParseFromArray(message, BUF_SIZE);

      workerAddr.sin_family = PF_INET;
      workerAddr.sin_port = htons(deserializedOperation.port());
      workerAddr.sin_addr.s_addr = inet_addr(deserializedOperation.ip().c_str());
      int sockWorker = socket(PF_INET, SOCK_STREAM, 0);
      if (connect(sockWorker, (struct sockaddr *)&workerAddr, sizeof(workerAddr)) < 0)
      {
          perror("connect error");
          exit(-1);
      }

      operation.set_operation(dos::Operation::COMPUTE);
      dos::Operation::DistributeTask *task = operation.add_task();
      task->set_operation_num_type("int");
      task->set_operation_num_one("1");
      task->set_operation_num_two("9999");
      task->set_operation_label("+");
      operation.SerializeToArray(message, BUF_SIZE);

      if (send(sockWorker,message, strlen(message), 0) < 0)
      {
          printf("send msg erroro");
          return 0;
      }
      
      ret = recv(sockWorker, message, BUF_SIZE, 0);
      deserializedOperation.ParseFromArray(message, BUF_SIZE);
      cout << "计算结果:" << deserializedOperation.DebugString();
      close(sockWorker);
      sleep(1);
    }

}

