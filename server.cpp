#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>

using namespace std;

int getServerSocket(int port,string ip){
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        cout << "socket 创建失败： "<< endl;
        exit(1);
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);//将一个无符号短整型的主机数值转换为网络字节顺序，即大尾顺序(big-endian)
    addr.sin_addr.s_addr = inet_addr(ip.c_str());//net_addr方法可以转化字符串，主要用来将一个十进制的数转化为二进制的数，用途多于ipv4的IP转化。
    bool nOptval = true;
    setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&nOptval,sizeof(bool));
    int res = bind(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
    if (res == -1)
    {
        cout << "bind创建失败： " << endl;
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
    int socket_fd = getServerSocket(8887,"127.0.0.1");
    cout << "bind ok 等待客户端的连接" << endl;
    //4.监听客户端listen()函数
    //参数二：进程上限，一般小于30
    listen(socket_fd,30);
    //5.等待客户端的连接accept()，返回用于交互的socket描述符
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int fd = accept(socket_fd,(struct sockaddr*)&client,&len);
    if (fd == -1)
    {
        cout << "accept错误\n" << endl;
        exit(-1);
    }
    //6.使用第5步返回socket描述符，进行读写通信。
    char *ip = inet_ntoa(client.sin_addr);
    cout << "客户： 【" << ip << "】连接成功" << endl;

    write(fd, "welcome", 7);
    fd_set fds;
    timeval timeout = {0};
    timeout.tv_usec = 500;
    char recv_buff[255];
    while(1){
        if(mySelect(fds,fd,500)){
            ssize_t recv_size = read(fd,recv_buff,255);
            printf("%s\n",recv_buff);
            memset(recv_buff,0,sizeof(recv_buff));
        }

        if(mySelect(fds,STDIN_FILENO,500)){
            ssize_t recv_size = read(STDIN_FILENO,recv_buff,255);
            if(send(fd,recv_buff,sizeof(recv_buff),0)<=0){
                cout<<"write error\n";
                exit(1);
            }
            memset(recv_buff,0,sizeof(recv_buff));
        }
    }
    //7.关闭sockfd
    close(fd);
    close(socket_fd);
    return 0;
}