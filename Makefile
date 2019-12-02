all:
	g++ -O0 -std=c++11 master.cpp utility.h -o master
	g++ -O0 -std=c++11 worker.cpp utility.h -o worker
