#include"SocketManager.h"
using namespace std;

long timeStamp(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec; 
}

string getTime(){
    time_t timep;
    time(&timep);
    char tmp[64];
    memset(tmp,0,sizeof(tmp));
    strftime(tmp,sizeof(tmp),"%Y-%m-%d %H:%M:%S ",localtime(&timep));
    return tmp;
}
int getSocket(int port,std::string ip,bool isServer){
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
    int res;
    if(!isServer){
        res = connect(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
    }else{
        res = bind(socket_fd,(struct sockaddr*)&addr,sizeof(addr));
        listen(socket_fd,30);
    }
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

vector<string> splitString(string s,char flag){
    vector<string> ret;

    for(int i = 0;i<s.size();i++){
        if(s[i] == flag){
            ret.push_back(s.substr(0,i));
            ret.push_back(s.substr(i+1,s.size()-i-1));
            return ret;
        }
    }
    return ret;
}