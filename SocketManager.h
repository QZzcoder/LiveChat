#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<cstring>
#include<set>
#include<time.h>
#include<vector>
#include<sys/time.h>

std::string getTime();
int getSocket(int port,std::string ip,bool isServer);
int mySelect(fd_set& fds,std::set<int> fd,int usec);
long timeStamp();
std::vector<std::string> splitString(std::string s,char flag);