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
#include "tdpool.h"
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

void GetData(int sclient){
    char revData[255];
    int ret = recv(sclient, revData, 255, 0);
    cout << "接收到数据了,来自: " << sclient  << endl;
    if (ret > 0)
    {
        // revData[ret] = 0x00;
        cout<< revData <<endl;
    }
    else exit(1);

    //如果收到byebye则结束聊天
    //发送数据  
    if(!strcmp(revData, "byebye")){ 
        const char* endData = "byebye";
        send(sclient, endData, strlen(endData), 0);
        auto pos = find(conns.begin(), conns.end(), sclient);
        conns.erase(pos);
        n--;
        close(sclient);
        FD_CLR(sclient,&rfds);
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
    FD_SET(slisten, &rfds);
    FD_SET(0, &rfds);
    sockaddr_in remoteAddr;
    socklen_t nAddrlen = sizeof(remoteAddr);
    ThreadPool tp;
    maxfd = slisten;
    while (true)  //循环接受数据和发送数据
    {
        fd_set temp = rfds;
        //接收数据  
        int res = select(maxfd+1, &temp, NULL, NULL, &tv);
        if(res == -1){
            cout << "select error" << endl;
            
        }
        else if(res == 0){
            // cout << "not message" <<endl;
            continue;
        }
        else{
            cout << res << 1;
            if(FD_ISSET(slisten, &temp)){
                sockaddr_in remoteAddr;
                socklen_t nAddrlen = sizeof(remoteAddr);
                int sClient = accept(slisten, (sockaddr *)&remoteAddr, &nAddrlen);
                cout << "有一个新连接:" << sClient << endl;
                if(n+1 > MAX_CANON) continue;
                n++;
                conns.push_back(sClient);
                maxfd = max(maxfd, sClient);
                FD_SET(sClient, &rfds);
    
            }
            for(int i = 0; i < conns.size(); i++){
                if(FD_ISSET(conns[i], &temp)){
                    pair<ThreadPool::func, int> f(GetData, conns[i]);
                    ThreadPool::task b(ThreadPool::low, f);
                    tp.AddTask(b);
                    cout << "加入连接" << conns[i] << "的任务" <<endl; 
                }
                if(FD_ISSET(0, &temp)){
                    char buf[255];
                    cin.getline(buf, 255);;//每次读取一行数据存放在buf中
                    // printf("you are send %s", buf);
                    for(int i=0; i<conns.size(); i++)
                    {
                        send(conns[i], buf, sizeof(buf), 0);
                    }   
                }
                
            }

        }
                //输入输出描述是就绪状态
        
        }


    // FD_ZERO(&rfds);
    // thread t1(Accpt);
    // thread t2(GetData);
    // thread t3(sendData);
    // t1.join();
    // t2.join();
    // t3.join();


 
    //循环接收数据  
    

    
    close(slisten);
    return 0;
}