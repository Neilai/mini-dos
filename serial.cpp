#include <iostream>
#include <string>
#include "mini_dos.pb.h"
using namespace std;

int main(int argc, char* argv[]){
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    // set message class operation
    tutorial::Operation operation;

    // set operation type
    operation.set_operation(tutorial::Operation::REGISTER);

    // set port id
    operation.set_port(8888);

    // set query 
    operation.set_query(true);

    // add a distribute task obj
    tutorial::Operation::DistributeTask* task = operation.add_task();
    task->set_operation_num_type("int");
    task->set_operation_num_one("1");
    task->set_operation_num_two("9999");
    task->set_operation_label("+");

    //  add a result obj
    tutorial::Operation::Result* result = operation.add_result();
    result->set_result_type("int");
    result->set_result_value("10000000");

    // set bool task_finished
    operation.set_task_finished(true);

    // deserialization below
    // serialize class operation to string container
    string serializedStr;
    operation.SerializeToString(&serializedStr);
    cout<<"serialization result:"<<serializedStr<<endl;
    cout<<endl<<"debugString:\n"<<operation.DebugString();


    tutorial::Operation deserializedOperation;
    if(!deserializedOperation.ParseFromString(serializedStr)){
        cerr << "Failed to parse student." << endl;
        return -1;
    }

    cout<<"-------------上面是序列化，下面是反序列化---------------"<<endl;
    cout<<"deserializedOperation debugString:"<<deserializedOperation.DebugString();
    
    cout<<endl<<"Operation Type: " << deserializedOperation.operation() << endl;
    cout<<endl<<"Port ID: " << deserializedOperation.port() << endl;
    cout<<endl<<"QUERY status: " << deserializedOperation.query() << endl;
    if(deserializedOperation.task_size()) {
        const tutorial::Operation::DistributeTask& task_content = deserializedOperation.task(0);
        cout<<"DistributeTask:"<<task_content.operation_num_type()<<endl;
        cout<<task_content.operation_num_one()<<task_content.operation_label()<<task_content.operation_num_two()<<endl;
    }
    if(deserializedOperation.result_size()) {
        const tutorial::Operation::Result& result_content = deserializedOperation.result(0);
        cout<<"Result:"<<endl;
        cout<<result_content.result_type()<<result_content.result_value()<<endl;
    }

    cout<<endl<<"task_finished status: " << deserializedOperation.task_finished() << endl;

    google::protobuf::ShutdownProtobufLibrary();
}
