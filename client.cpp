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


using namespace std;
int getClientSocket(int port,string ip){
    int socket_fd = socket(AF_INET, SOCK_STREAM,0);
    if(socket_fd == -1)
    {
        cout<<"socket 创建失败："<<endl;
        exit(-1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    bool nOptval = true;
    setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&nOptval,sizeof(bool));
    int res = connect(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
    if(res == -1){
        cout<<"bind 链接失败："<<endl;
        exit(-1);
    }
    return socket_fd;
}
bool mySelect(fd_set& fds,int fd,int usec){
    timeval timeout = {0};
    timeout.tv_usec = usec;
    FD_ZERO(&fds);
    FD_SET(fd,&fds);
    switch(select(fd+1,&fds,NULL,NULL,&timeout)){
        case -1:{
            exit(1);
        }case 0:break;
        default:{
            if(FD_ISSET(fd,&fds)){
                return true;
            }
        }
    }
    return false;
}
int main()
{
    int socket_fd = getClientSocket(8887,"127.0.0.1");

    write(socket_fd,"hello hebinbing",15);
    fd_set fds;
    timeval timeout = {0};
    timeout.tv_usec = 500;
    char recv_buff[255];
    while(1){
        if(mySelect(fds,socket_fd,500)){
            ssize_t recv_size = read(socket_fd,recv_buff,255);
            printf("%s\n",recv_buff);
            memset(recv_buff,0,sizeof(recv_buff));
        }
        if(mySelect(fds,STDIN_FILENO,500)){
            ssize_t recv_size = read(STDIN_FILENO,recv_buff,255);
            if(send(socket_fd,recv_buff,sizeof(recv_buff),0)<=0){
                cout<<"write error\n";
                exit(1);
            }
            memset(recv_buff,0,sizeof(recv_buff));
        }
    }

    close(socket_fd);

    return 0;
}