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

using namespace std;
string getTime(){
    time_t timep;
    time(&timep);
    char tmp[64];
    memset(tmp,0,sizeof(tmp));
    strftime(tmp,sizeof(tmp),"%Y-%m-%d %H:%M:%S ",localtime(&timep));
    return tmp;
}
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
    string name;
    cin>>name;
    cout<<"Hello "<<name<<endl;
    int socket_fd = getClientSocket(8888,"127.0.0.1");
    write(socket_fd,name.c_str(),name.size());
    fd_set fds;
    char recv_buff[255];
    set<int> clientSelectSet,cinSelectSet;
    clientSelectSet.insert(socket_fd);
    cinSelectSet.insert(STDIN_FILENO);
    while(1){
        if(mySelect(fds,clientSelectSet,500) == socket_fd){
            ssize_t recv_size = read(socket_fd,recv_buff,255);
            cout<<getTime();
            if(recv_size<=0){
                cout<<"shutdown"<<endl;
                break;
            }
            if(recv_buff[0] == 'N'){
                cout<<"sign in fail"<<endl;
                break;
            }
            printf("%s",recv_buff);
            memset(recv_buff,0,sizeof(recv_buff));
        }
        if(mySelect(fds,cinSelectSet,500) == STDIN_FILENO){
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