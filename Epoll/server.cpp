#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <thread>
#include <iostream>
#include <vector>
#include <utils.h>
#define MAXCONN 5 //最大连接数

// #pragma comment(lib,"ws2_32.lib")  
using namespace std;

set<int> conns;
int slisten;
int n = 0;
struct epoll_event epevs[1024];

void Accpt(){
    sockaddr_in remoteAddr;
    socklen_t nAddrlen = sizeof(remoteAddr);
    while(true){
        int sClient = accept(slisten, (sockaddr *)&remoteAddr, &nAddrlen);
        cout << "有一个新连接:" << sClient << endl;
        if(n+1 > MAX_CANON) continue;
        n++;
        conns.insert(sClient);
        addfd(epfd , sClient, 1);
        
    }
} 

void GetData(){
    int sClient;
    char revData[255];
    while (true)  //循环接受数据和发送数据
    {
        memset(revData, 0, 255);
        char sendData[255];
        fd_set temp = rfds;
        //接收数据  
        int res = epoll_wait(epfd, epevs, 1024, -1);
        if(res == -1){
            cout << "select error" << endl;
        }
        else if(res == 0){
            cout << "not message" <<endl;
        }
        else{
            for(int i = 0; i < epevs.size(); i++){
                if(epevs[i].events & EPOLLOUT)
                {
                    continue;
                }
                int curfd = epevs[i].data.fd;
                int ret = recv(curfd, revData, 255, 0);
                cout << "接收到数据了,来自: " << curfd  << endl;
                if (ret > 0)
                {
                    // revData[ret] = 0x00;
                    cout<< revData <<endl;
                }
                else if(ret == 0){
                    cout<< curfd << "的连接关闭" <<endl;
                    epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, &epevs[i]);
                    close(curfd)
                    conns.erase(curfd);
                    n--;
                }
                else exit(1);
              
                //如果收到byebye则结束聊天
                //发送数据  
                if(!strcmp(revData, "byebye")){ 
                    const char* endData = "byebye";
                    send(curfd, endData, strlen(endData), 0);
                    n--;
                    close(conns[i]);
                    conns.erase(curfd);
                }

            }
        }
    }
}

void sendData(){
    while(true){
        char sendData[255];
        cin.getline(sendData, 255);
        for(auto i = conns.begin(); i != conns.end();i++){
            send(*i, sendData, sizeof(sendData), 0);
        }
    }
}

int main(int argc, char* argv[]){
    slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//返回一个套接字描述符 
    sockaddr_in server_socket_addr;
    server_socket_addr.sin_family = AF_INET;
    server_socket_addr.sin_port = htons(8017);
    server_socket_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (::bind(slisten, (sockaddr*)&server_socket_addr, sizeof(server_socket_addr)) == int(-1))
    {
        cout << "bind error !" << endl;
        close(slisten);
        exit(1);
    }
 
    //开始监听  
    if (listen(slisten, 5) == -1)
    {
        cout << "listen error !" << endl;
        close(slisten);
        exit(1);
    }
    else cout << "server starts up" << endl;
    int epfd = epoll_create(128);
    addfd(epfd , slisten, 1);
    thread t1(Accpt);
    thread t2(GetData);
    thread t3(sendData);
    t1.join();
    t2.join();
    t3.join();


 
    //循环接收数据  
    

    
    close(slisten);
    return 0;
}