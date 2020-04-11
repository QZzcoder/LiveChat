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
#include<deque>
#include<mutex>
#include<thread>
#include<condition_variable>

std::string getTime();
int getSocket(int port,std::string ip,bool isServer);
int mySelect(fd_set& fds,std::set<int> fd,int usec);
long timeStamp();
std::vector<std::string> splitString(std::string s,char flag);


template<typename T>
class Blockqueue{
public:
    Blockqueue():_mutex(),_cv(),_queue(){}

    void put(const T& t);
    
    T take();

private:
    std::mutex _mutex;
    std::condition_variable _cv;
    std::deque<T> _queue;
};


template<typename T>
void Blockqueue<T>::put(const T& t){
    std::unique_lock<std::mutex> lock(_mutex);
    _queue.push_back(t);
    _cv.notify_one();
}
template<typename T>
T Blockqueue<T>::take(){
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock,[this]{return !this->_queue.empty();});
    T ret(_queue.front());
    _queue.pop_front();
    return ret;
}