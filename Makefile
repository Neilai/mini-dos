OBJS= worker.o master.o client.o
TARGETS = $(patsubst %.o,%,${OBJS})
all:$(TARGETS) 
$(TARGETS):%:%.o
	g++   -std=c++11  -lprotobuf -lpthread $<  mini_dos.pb.o -o $@
$(OBJS):%.o:%.cpp
	g++  -std=c++11   -c $<  mini_dos.pb.cc  
proto:
	protoc mini_dos.proto --cpp_out=./
test:
	g++  -std=c++11  -lprotobuf -lpthread serial.cpp mini_dos.pb.cc  -o test
clean:
	rm  -rf  *.o  ${TARGETS}
