# mini-dos homework

1. mini_dos.proto协议文件编译

   `protoc mini_dos.proto --cpp_out=./`

2. serial.cpp序列化反序列化文件编译

   ```g++ mini_dos.pb.cc serial.cpp -o serial `pkg-config --cflags --libs protobuf` -lpthread -std=c++11```

