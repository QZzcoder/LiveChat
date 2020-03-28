#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<iostream>
#include<set>
#include<map>
#include<vector>
#include<time.h>

using namespace std;
vector<string> splitNameMsg(string s){
    vector<string> ret;
    for(int i = 0;i<s.size();i++){
        if(s[i] == '@'){
            ret.push_back(s.substr(0,i));
            ret.push_back(s.substr(i+1,s.size()-i));
            return ret;
        }
    }
    return ret;
}
string getTime(){
    time_t timep;
    time(&timep);
    char tmp[64];
    memset(tmp,0,sizeof(tmp));
    strftime(tmp,sizeof(tmp),"%Y-%m-%d %H:%M:%S ",localtime(&timep));
    return tmp;
}

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
int mySelect(fd_set& fds,set<int> fd,int usec){
    timeval timeout = {0};
    timeout.tv_usec = usec;
    FD_ZERO(&fds);
    int maxfd = 0;
    for(int i:fd){
        maxfd = maxfd>i?maxfd:i;
        FD_SET(i,&fds);
    }

    switch(select(maxfd+1,&fds,NULL,NULL,&timeout)){
        case -1:{
            cout<<"select error";
            exit(1);
        }case 0:break;
        default:{
            for(int i:fd){

                if(FD_ISSET(i,&fds)){
                    return i;
                }
            }
        }
    }
    return -1;
}
int main()
{
    int socket_fd = getServerSocket(8888,"127.0.0.1");
    cout << "bind ok 等待客户端的连接" << endl;
    //4.监听客户端listen()函数
    //参数二：进程上限，一般小于30
    listen(socket_fd,30);
    //5.等待客户端的连接accept()，返回用于交互的socket描述符
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    

    fd_set fds;
    map<string,int> name2sock;
    
    char recv_buff[255];
    set<int> serverSelectSet,clientSelectSet,cinSelectSet;
    serverSelectSet.insert(socket_fd);
    cinSelectSet.insert(STDIN_FILENO);
    while(1){

        if(mySelect(fds,serverSelectSet,500) == socket_fd){
            cout<<getTime()<<"User log!";
            int client_fd = accept(socket_fd,(struct sockaddr*)&client,&len);
            if (client_fd == -1)
            {
                cout << "accept错误\n" << endl;
                exit(-1);
            }
            char name[255];

            ssize_t recv_size = read(client_fd,name,255);
            clientSelectSet.insert(client_fd);
            name2sock[name] = client_fd;
            cout<<name<<endl;

        }

        int activeClient = mySelect(fds,clientSelectSet,500);
        if(activeClient > 0){
            ssize_t recv_size = read(activeClient,recv_buff,255);
            if(recv_size<=0){
                break;
            }
            vector<string> msg = splitNameMsg(recv_buff);
            if(msg.size() == 0){
                send(activeClient,"Server:@ to split name and msg\n",31,0);
                continue;
            }
            send(name2sock[msg[0]],msg[1].c_str(),msg[1].size(),0);
            cout<<getTime();
            printf("broadcast:%s",recv_buff);
            memset(recv_buff,0,sizeof(recv_buff));
        }

        if(mySelect(fds,cinSelectSet,500) == STDIN_FILENO){
            ssize_t recv_size = read(STDIN_FILENO,recv_buff,255);
            if(recv_buff[0] == 'e' &&recv_buff[1] == 'n' &&recv_buff[2] == 'd'){
                break;
            }
        }
    }
    //7.关闭sockfd
    close(socket_fd);
    return 0;
}