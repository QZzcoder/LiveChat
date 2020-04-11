CC = g++
cflags = -std=c++11
mysql = `mysql_config --cflags --libs`
thread = -pthread

all:server client

client:client.o SocketManager.o
	$(CC) -o client client.o SocketManager.o $(cflags) $(thread) 

server:server.o SocketManager.o
	$(CC) -o server server.cpp SocketManager.o $(cflags) $(mysql) $(thread) 

server.o:server.cpp
	$(CC) -c server.cpp $(cflags) $(mysql) $(thread) 

client.o:client.cpp
	$(CC) -c client.cpp $(cflags)

SocketManager.o:SocketManager.h SocketManager.cpp
	$(CC) -c SocketManager.cpp $(cflags)  $(thread) 