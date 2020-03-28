all:server client
server:server.cpp
	g++ -o server server.cpp -std=c++11

client:client.cpp
	g++ -o client client.cpp -std=c++11

