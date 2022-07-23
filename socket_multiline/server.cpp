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
#include <iostream>
#include <thread>
#define PORT 7000
#define QUEUE 20//连接请求队列
#define MAX_CONN 3


// #pragma comment(lib,"ws2_32.lib")  
using namespace std;

void process(int &sClient ,int id){
    // thread::id i = this_thread::get_id();
    char revData[255];
    while (true)  //循环接受数据和发送数据
    {
        memset(revData, 0, 255);
        char sendData[255];
        //接收数据  
        int ret = recv(sClient, revData, 255, 0);
        if (ret > 0)
        {
            // revData[ret] = 0x00;
            cout<< "来自bot" << id << "的留言: " << revData <<endl;
        }
        else exit(1);
        //如果收到byebye则结束聊天
        if(!strcmp(revData, "byebye")){ 
            const char* endData = "byebye";
            send(sClient, endData, strlen(endData), 0);
            break;
        }
        //发送数据 
        cout << "给bot"<< id << "回复:" << endl; 
        cin.getline(sendData, 255);
        send(sClient, sendData, strlen(sendData), 0);

    }
    close(sClient);
}
int main(){
    int slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//返回一个套接字描述符
    // if (slisten == INVALID_SOCKET)
    // {
    //     cout << "create socket error !" << endl;
    //     return 0;
    // }
    int sockets[MAX_CONN];
    //绑定IP和端口  
    int conn = 0;
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
    if (listen(slisten, 3) == -1)
    {
        cout << "listen error !" << endl;
        close(slisten);
        exit(1);
    }
 
    //循环接收数据  
    while(true){
        int sClient;
        sockaddr_in remoteAddr;
        socklen_t nAddrlen = sizeof(remoteAddr);
        char revData[255];
        cout << "阻塞。。。。等待连接。。。" << endl;
        sockets[conn] = accept(slisten, (sockaddr *)&remoteAddr, &nAddrlen);
        //接收数据  
        if (sClient < 0)
        {
            cout << "accept error !" << endl;
            close(slisten);
            return 0;
        }
        else  cout << "接受到一个连接：" << inet_ntoa(remoteAddr.sin_addr) << ": " << remoteAddr.sin_port << endl;
        if(conn >= MAX_CONN) break;
        int ret = recv(sockets[conn], revData, 255, 0);
        if (ret > 0)
        {
            // revData[ret] = 0x00;
            cout<< revData <<endl;
        }
        else{
            cout << "数据接收失败" << endl;
            continue;
        }

        //发送数据  
        const char * sendData = "你好，可以开始聊天了!";
        send(sockets[conn], sendData, strlen(sendData), 0);
        cout << sendData <<endl;
        thread t(process, ref(sockets[conn]), conn);
        t.detach();
        cout << 1 << endl;
        conn++;
    }
    close(slisten);
}