#include"SocketManager.h"
using namespace std;

int main()
{
    string name;
    cin>>name;
    cout<<"Hello "<<name<<endl;
    int socket_fd = getSocket(8888,"127.0.0.1",false);
    write(socket_fd,name.c_str(),name.size());
    fd_set fds;
    char recv_buff[255];
    set<int> clientSelectSet,cinSelectSet;
    clientSelectSet.insert(socket_fd);
    cinSelectSet.insert(STDIN_FILENO);
    while(1){
        if(mySelect(fds,clientSelectSet,0) == socket_fd){
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
        if(mySelect(fds,cinSelectSet,0) == STDIN_FILENO){
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