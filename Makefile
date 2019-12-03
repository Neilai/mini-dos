all:
	g++ -O1 -std=c++11 master.cpp utility.h -o master
	g++ -O1 -std=c++11 worker.cpp utility.h -o worker
	g++ -O1 -std=c++11 client.cpp utility.h -o client
clean:
	rm  -rf client master worker
