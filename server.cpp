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
#include<deque>
#include<mutex>
#include<thread>
#include<condition_variable>
#include"SocketManager.h"
using namespace std;

Blockqueue<int> bq;
int socket_fd;
struct sockaddr_in client;

fd_set fds;

map<string,int> name2sock;
set<int> serverSelectSet,clientSelectSet,cinSelectSet;
socklen_t len;
char dbpassword[] = "0520";//在此处更改密码

vector<string> getOldMsg(string name){
    vector<string> ret;
    MYSQL mysql_conn;
    MYSQL_RES * ptr_res;
    MYSQL_ROW result_row;
    mysql_init(&mysql_conn);
    if(!mysql_real_connect(&mysql_conn,"localhost","root",dbpassword,"Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    string sql = "select logofftime from User where name = \"" + name + "\";";
    if(!mysql_query(&mysql_conn,sql.c_str())){
        ptr_res = mysql_use_result(&mysql_conn);
        if(ptr_res){
            result_row = mysql_fetch_row(ptr_res);
        }
    }
    string logofftime = result_row[0];

    mysql_init(&mysql_conn);
    if(!mysql_real_connect(&mysql_conn,"localhost","root",dbpassword,"Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    sql = "select * from message where target = \"" + name + "\" and time > "+logofftime+";";
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
    if(!mysql_real_connect(&mysql_conn,"localhost","root",dbpassword,"Chat",0,NULL,0)){
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
    if(!mysql_real_connect(&mysql_conn,"localhost","root",dbpassword,"Chat",0,NULL,0)){
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
    if(!mysql_real_connect(&mysql_conn,"localhost","root",dbpassword,"Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    string sql = "select * from User where name = \"" + name + "\" and password = "+password+";";
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
    if(!mysql_real_connect(&mysql_conn,"localhost","root",dbpassword,"Chat",0,NULL,0)){
        cout<<"mysql error";
        exit(1);
    }
    string sql = "select * from User where name = \"" + name + "\"";
    mysql_query(&mysql_conn,sql.c_str());
    ptr_res = mysql_store_result(&mysql_conn);
    if(!ptr_res){return false;}

    sql = "insert into User values(\""+name+"\",\""+password+"\",0);";
    return !mysql_query(&mysql_conn,sql.c_str());
}


void oneLoop(){
    while(1){
        char recv_buff[255];
        int sock = bq.take();
        if(sock == -404){
            break;
        }else if(sock == socket_fd){
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
            ::memset(signMsg,0,255);

            if(vec.size() != 2 ||!userCheck(vec[0],vec[1])){//登录失败
                send(client_fd,"N",1,0);
                cout<<" fail\n";
                close(client_fd);
            }else{
                cout<<vec[0]<<endl;
                cout<<"old msg to "<<vec[0]<<endl;
                vector<string> oldMsg = getOldMsg(vec[0]);
                for(string s:oldMsg){
                    cout<<"    "<<s;
                    send(client_fd,s.c_str(),s.size(),0);
                }
                clientSelectSet.insert(client_fd);
                name2sock[vec[0]] = client_fd;
            }
        }else if(clientSelectSet.count(sock) == 1){
            int activeClient = sock;
            string oriName;
            for(auto it:name2sock){
                if(it.second == activeClient){
                    oriName = it.first;
                    break;
                }
            }
            ssize_t recv_size = read(activeClient,recv_buff,255);
            if(recv_size<=0){//客户端退出
                updateTime(oriName);
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
            ::printf("message:%s",recv_buff);
            ::memset(recv_buff,0,sizeof(recv_buff));
        }
    }
}

int main()
{
    int shutDownNum = -404;
    socket_fd = getSocket(8888,"127.0.0.1",true);
    cout << "bind ok 等待客户端的连接" << endl;
    cout << "新建用户: 用户名&密码" << endl;
    cout << "退出: end" << endl;

    len = sizeof(client);

    char recv_buff[255];

    serverSelectSet.insert(socket_fd);
    cinSelectSet.insert(STDIN_FILENO);
    thread t1(oneLoop);
    thread t2(oneLoop);
    thread t3(oneLoop);
    while(1){
        //处理登录
        if(mySelect(fds,serverSelectSet,0) == socket_fd){
            bq.put(socket_fd);
        }
        //一次收发
        int activeClient = mySelect(fds,clientSelectSet,0);
        if(clientSelectSet.count(activeClient) == 1){
            bq.put(activeClient);

        }
        //服务器控制
        if(mySelect(fds,cinSelectSet,500) == STDIN_FILENO){
            ssize_t recv_size = read(STDIN_FILENO,recv_buff,255);
            if(recv_buff[0] == 'e' &&recv_buff[1] == 'n' &&recv_buff[2] == 'd'){
                cout<<getTime()<<"server Shutdown"<<endl;
                for(auto it:name2sock){
                    cout<<it.first<<" shutdown"<<endl;
                    close(it.second);
                }
                for(int i = 0;i<3;i++){
                    bq.put(shutDownNum);
                }

                break;
            }
            vector<string> msg = splitString(recv_buff,'&');
            if(msg.size() == 2&&newUser(msg[0],msg[1])){
                cout<<getTime()<<"新用户添加成功:"<<msg[0]<<"--"<<msg[1];
            }else{
                cout<<"用户名和密码间隔 : &"<<endl;
            }
        }
    }
    //7.关闭sockfd
    t1.join();
    t2.join();
    t3.join();
    close(socket_fd);
    return 0;
}