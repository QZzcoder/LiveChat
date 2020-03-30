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
#include<sys/time.h>
#include<mysql/mysql.h>

using namespace std;
long timeStamp(){
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec; 
}
vector<string> getOldMsg(string name){
    vector<string> ret;
    MYSQL mysql_conn;
    MYSQL_RES * ptr_res;
    MYSQL_ROW result_row;
    mysql_init(&mysql_conn);
    if(!mysql_real_connect(&mysql_conn,"localhost","root","0520","Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    string sql = "select logofftime from User where name = \"" + name + "\";";
    if(!mysql_query(&mysql_conn,sql.c_str())){
        ptr_res = mysql_use_result(&mysql_conn);
        if(ptr_res){
            result_row = mysql_fetch_row(ptr_res);

            // while((result_row = mysql_fetch_row(ptr_res))!=NULL){
            //     for(int i = 0;i<mysql_num_fields(ptr_res);i++){
            //         cout<<atol(result_row[i]);
            //     }
            //     cout<<endl;
            // }
        }
    }
    string logofftime = result_row[0];

    mysql_init(&mysql_conn);
    if(!mysql_real_connect(&mysql_conn,"localhost","root","0520","Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    sql = "select * from message where target = \"" + name + "\" and time > "+logofftime+";";
    cout<<sql;
    if(!mysql_query(&mysql_conn,sql.c_str())){
        ptr_res = mysql_use_result(&mysql_conn);
        if(ptr_res){
            while((result_row = mysql_fetch_row(ptr_res))!=NULL){
                vector<string> temp(4,"");
                for(int i = 0;i<mysql_num_fields(ptr_res);i++){
                    temp[i] = result_row[i];
                }
                ret.push_back(temp[1]+":"+temp[3]);
            }
        }
    }
    mysql_close(&mysql_conn);

    return ret;
}
bool storeMessage(string origin,string target,string msg){
    MYSQL mysql_conn;
    MYSQL_RES * ptr_res;
    MYSQL_ROW result_row;
    mysql_init(&mysql_conn);
    if(!mysql_real_connect(&mysql_conn,"localhost","root","0520","Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    string sql = "insert into message values("+to_string(timeStamp())
                +",\'"+origin+"\',\'"+target+"\',\'"+msg+"\');";

    return !mysql_query(&mysql_conn,sql.c_str());
}
bool updateTime(string name){
    string sql = "update User set logofftime = "+to_string(timeStamp())+" where name =\""+name+"\";";
    MYSQL mysql_conn;
    MYSQL_RES * ptr_res;
    MYSQL_ROW result_row;
    mysql_init(&mysql_conn);
    if(!mysql_real_connect(&mysql_conn,"localhost","root","0520","Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    if(!mysql_query(&mysql_conn,sql.c_str())){
        return true;
    }
    return false;
}

bool userCheck(string name,string password){
    MYSQL mysql_conn;
    MYSQL_RES * ptr_res;
    MYSQL_ROW result_row;
    mysql_init(&mysql_conn);
    if(!mysql_real_connect(&mysql_conn,"localhost","root","0520","Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    string sql = "select * from User where name = \"" + name + "\" and password = "+password+";";
    cout<<sql;
    if(!mysql_query(&mysql_conn,sql.c_str())){
        ptr_res = mysql_use_result(&mysql_conn);
        return mysql_fetch_row(ptr_res)!=NULL;
    }
    cout<<"query error"<<endl;
    mysql_close(&mysql_conn);
    return false;
}

bool newUser(string name,string password){
    MYSQL mysql_conn;
    MYSQL_RES * ptr_res;
    MYSQL_ROW result_row;
    mysql_init(&mysql_conn);
    if(!mysql_real_connect(&mysql_conn,"localhost","root","0520","Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    string sql = "select * from User where name = \"" + name + "\"";
    mysql_query(&mysql_conn,sql.c_str());
    ptr_res = mysql_store_result(&mysql_conn);
    if(!ptr_res){return false;}

    sql = "insert into User values(\""+name+"\",\""+password+"\",0);";
    cout<<sql;
    return !mysql_query(&mysql_conn,sql.c_str());
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

    char recv_buff[255];
    map<string,int> name2sock;
    set<int> serverSelectSet,clientSelectSet,cinSelectSet;
    serverSelectSet.insert(socket_fd);
    cinSelectSet.insert(STDIN_FILENO);
    while(1){
        //处理登录
        if(mySelect(fds,serverSelectSet,0) == socket_fd){
            cout<<getTime()<<"User sign up：";
            int client_fd = accept(socket_fd,(struct sockaddr*)&client,&len);
            if (client_fd == -1)
            {
                cout << "accept错误\n" << endl;
                exit(-1);
            }
            char signMsg[255];

            ssize_t recv_size = read(client_fd,signMsg,255);
            vector<string> vec = splitString(signMsg,'@');
            memset(signMsg,0,255);

            if(vec.size() != 2 ||!userCheck(vec[0],vec[1])){//登录失败
                send(client_fd,"N",1,0);
                cout<<" fail\n";
                close(client_fd);
            }else{
                cout<<vec[0]<<endl;
                cout<<"old msg to "<<vec[0]<<endl;
                vector<string> oldMsg = getOldMsg(vec[0]);
                for(string s:oldMsg){
                    cout<<"    "<<s<<endl;
                    send(client_fd,s.c_str(),s.size(),0);
                }
                clientSelectSet.insert(client_fd);
                name2sock[vec[0]] = client_fd;
            }
        }
        //一次收发
        int activeClient = mySelect(fds,clientSelectSet,0);
        if(activeClient > 0){
            string oriName;
            for(auto it:name2sock){
                if(it.second == activeClient){
                    oriName = it.first;
                    break;
                }
            }
            ssize_t recv_size = read(activeClient,recv_buff,255);
            if(recv_size<=0){
                updateTime(oriName);
                cout<<oriName<<" shutdown"<<endl;
                clientSelectSet.erase(name2sock[oriName]);
                name2sock.erase(oriName);
                continue;
            }
            vector<string> msg = splitString(recv_buff,'>');
            if(msg.size() != 0){
                storeMessage(oriName,msg[0],msg[1]);
                msg[1] = oriName + ":" + msg[1];
                send(name2sock[msg[0]],msg[1].c_str(),msg[1].size(),0);
            }
            cout<<getTime();
            printf("message:%s",recv_buff);
            memset(recv_buff,0,sizeof(recv_buff));
        }
        //服务器控制
        if(mySelect(fds,cinSelectSet,500) == STDIN_FILENO){
            ssize_t recv_size = read(STDIN_FILENO,recv_buff,255);
            cout<<recv_buff;
            if(recv_buff[0] == 'e' &&recv_buff[1] == 'n' &&recv_buff[2] == 'd'){
                cout<<getTime()<<"server Shutdown"<<endl;
                for(auto it:name2sock){
                    cout<<it.first<<" shutdown"<<endl;
                    close(it.second);
                }

                break;
            }
            vector<string> msg = splitString(recv_buff,'&');
            if(msg.size() == 2&&newUser(msg[0],msg[1])){
                cout<<getTime()<<"insert complete:"<<msg[0]<<":"<<msg[1];
            }else{
                cout<<"split : &"<<endl;
            }
        }
    }
    //7.关闭sockfd
    close(socket_fd);
    return 0;
}