# mini-dos

#### 已经完成的功能：

- worker启动向master注册
- master维护worker信息队列
- worker下线master自动检测
- client请求master分配worker
- master选择负荷最小的worker
- client请求计算
- worker完成计算，支持四则运算，三种类型
- worker，master使用epoll支持高并发
#### 使用：

```shell
make
./master (默认监听8888)
./worker 8889 (第二个参数是监听端口，可以启动多个worker)
./worker 8890
./worker 8891
./client (启动多个client)
./client
./client

```

