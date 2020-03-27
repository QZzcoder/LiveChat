#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>

using namespace std;

int main()
{
    //1.创建一个socket
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        cout << "socket 创建失败： "<< endl;
        exit(1);
    }
    //2.准备通讯地址（必须是服务器的）192.168.1.49是本机的IP
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8887);//将一个无符号短整型的主机数值转换为网络字节顺序，即大尾顺序(big-endian)
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");//net_addr方法可以转化字符串，主要用来将一个十进制的数转化为二进制的数，用途多于ipv4的IP转化。
    //3.bind()绑定
    //参数一：0的返回值（socket_fd）
    //参数二：(struct sockaddr*)&addr 前面结构体，即地址
    //参数三: addr结构体的长度
    bool nOptval = true;
    setsockopt(socket_fd,SOL_SOCKET,SO_REUSEADDR,(const void*)&nOptval,sizeof(bool));
    int res = bind(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
    if (res == -1)
    {
        cout << "bind创建失败： " << endl;
        exit(-1);
    }
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
   
    char buffer[255]={};
    int size = read(fd, buffer, sizeof(buffer));//通过fd与客户端联系在一起,返回接收到的字节数
    //第一个参数：accept 返回的文件描述符
    //第二个参数：存放读取的内容
    //第三个参数：内容的大小

    cout << "接收到字节数为： " << size << endl;
    cout << "内容： " << buffer << endl;

    //7.关闭sockfd
    close(fd);
    close(socket_fd);
    return 0;
}