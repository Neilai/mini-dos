#include <iostream>
#include <string>
#include "mini_dos.pb.h"
using namespace std;

int main(int argc, char* argv[]){
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    // set message class operation
    dos::Operation operation;

    // set operation type
    operation.set_operation(dos::Operation::REGISTER);

    // set port id
    operation.set_port(8888);

    // add a distribute task obj
    dos::Operation::DistributeTask* task = operation.add_task();
    task->set_operation_num_type("int");
    task->set_operation_num_one("1");
    task->set_operation_num_two("9999");
    task->set_operation_label("+");

    //  add a result obj
    //dos::Operation::Result* result = operation.add_result();
    //result->set_result_type("int");
    //result->set_result_value("10000000");

    // deserialization below
    // serialize class operation to string container
    char *serializedStr[1000];
    operation.SerializeToArray(serializedStr,1000);
    //cout<<"serialization result:"<<serializedStr<<endl;
    cout<<endl<<"debugString:\n"<<operation.DebugString();


    dos::Operation deserializedOperation;
    deserializedOperation.ParseFromArray(serializedStr,1000);

    cout<<"-------------上面是序列化，下面是反序列化---------------"<<endl;
    cout<<"deserializedOperation debugString:"<<deserializedOperation.DebugString();
    
    cout<<endl<<"Operation Type: " << deserializedOperation.operation() << endl;
    cout<<endl<<"Port ID: " << deserializedOperation.port() << endl;
    //if(deserializedOperation.task_size()) {
        const dos::Operation::DistributeTask& task_content = deserializedOperation.task(0);
        cout<<"DistributeTask:"<<task_content.operation_num_type()<<endl;
        cout<<task_content.operation_num_one()<<task_content.operation_label()<<task_content.operation_num_two()<<endl;
    //}
    //if(deserializedOperation.result_size()) {
        //const dos::Operation::Result& result_content = deserializedOperation.result(0);
        //cout<<"Result:"<<endl;
        //cout<<result_content.result_type()<<result_content.result_value()<<endl;
    //}


    google::protobuf::ShutdownProtobufLibrary();
}
