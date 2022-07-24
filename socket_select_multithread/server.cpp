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
#define MAXCONN 5 //最大连接数

// #pragma comment(lib,"ws2_32.lib")  
using namespace std;

vector<int> conns;
int slisten;
int maxfd;
fd_set rfds;   
int retval = 0;
int n = 0;
struct timeval tv;

void Accpt(){
    sockaddr_in remoteAddr;
    socklen_t nAddrlen = sizeof(remoteAddr);
    while(true){
        int sClient = accept(slisten, (sockaddr *)&remoteAddr, &nAddrlen);
        cout << "有一个新连接:" << sClient << endl;
        if(n+1 > MAX_CANON) continue;
        n++;
        conns.push_back(sClient);
        maxfd = max(maxfd, sClient);
        FD_SET(sClient, &rfds);
    }
} 

void GetData(){
    int sClient;
    tv.tv_sec = 10;//设置倒计时时间
    tv.tv_usec = 0;
    sockaddr_in remoteAddr;
    socklen_t nAddrlen = sizeof(remoteAddr);
    char revData[255];
    while (true)  //循环接受数据和发送数据
    {
        memset(revData, 0, 255);
        char sendData[255];
        fd_set temp = rfds;
        //接收数据  
        int res = select(maxfd+1, &temp, NULL, NULL, &tv);
        if(res == -1){
            cout << "select error" << endl;
        }
        else if(res == 0){
            cout << "not message" <<endl;
        }
        else{
            for(int i = 0; i < conns.size(); i++){
                if(FD_ISSET(conns[i], &temp)){
                    int ret = recv(conns[i], revData, 255, 0);
                    cout << "接收到数据了,来自: " << conns[i]  << endl;
                    if (ret > 0)
                    {
                        // revData[ret] = 0x00;
                        cout<< revData <<endl;
                    }
                    else exit(1);
                }
                //如果收到byebye则结束聊天
                //发送数据  
                if(!strcmp(revData, "byebye")){ 
                    const char* endData = "byebye";
                    send(conns[i], endData, strlen(endData), 0);
                    conns.erase(conns.begin()+i);
                    n--;
                    close(conns[i]);
                    FD_CLR(conns[i],&rfds);
                }

            }
        }
    }
}

void sendData(){
    while(true){
        char sendData[255];
        cin.getline(sendData, 255);
        for(int i = 0; i< conns.size();i++){
            send(conns[i], sendData, sizeof(sendData), 0);
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
    FD_ZERO(&rfds);
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