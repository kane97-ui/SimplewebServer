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
#define PORT 7000
#define QUEUE 20//连接请求队列


// #pragma comment(lib,"ws2_32.lib")  
using namespace std;
 
int main(int argc, char* argv[])
{
    
    int slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//返回一个套接字描述符 
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
 
    //循环接收数据  
    int sClient;
    sockaddr_in remoteAddr;
    socklen_t nAddrlen = sizeof(remoteAddr);
    char revData[255];
    fd_set rfds;   
    FD_ZERO(&rfds);
    int maxfd = slisten;
    int retval = 0;
    FD_SET(slisten, &rfds);
    FD_SET(0, &rfds);
    struct timeval tv;
    tv.tv_sec = 10;//设置倒计时时间
    tv.tv_usec = 0;
    vector<int> cons;
    cout << "阻塞。。。。等待连接。。。" << endl;
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
            if(FD_ISSET(slisten, &temp)){
                sClient = accept(slisten, (sockaddr *)&remoteAddr, &nAddrlen);
                FD_SET(sClient, &rfds);
                cons.push_back(sClient);
                cout << "有一个新的连接:" << sClient << endl;
                maxfd = maxfd<sClient? sClient:maxfd;
            }
            for(int i = 0; i < cons.size(); i++){
                if(FD_ISSET(cons[i], &temp)){
                    int ret = recv(cons[i], revData, 255, 0);
                    cout << "接收到数据了,来自: " << cons[i]  << endl;
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
                    send(cons[i], endData, strlen(endData), 0);
                    cons.erase(cons.begin()+i);
                    close(cons[i]);
                }
                //输入输出描述是就绪状态
                if(FD_ISSET(0, &temp)){
                    cin.getline(sendData, 255);;//每次读取一行数据存放在buf中
                    //printf("you are send %s", buf);
                    for(int i=0; i<cons.size(); i++)
                    {
                        send(cons[i], sendData, sizeof(sendData), 0);
                    }   
                }

            }
        }

    }
    close(slisten);
    return 0;
}