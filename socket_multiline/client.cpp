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
#include<iostream>
 
#define MYPORT  7000

 
#pragma comment(lib,"ws2_32.lib")  
using namespace std;
 
int main()
{

    int sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(8017);
    serAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == -1)
    {  
        //连接失败 
        cout << "connect error !" << endl;
        close(sclient);
        exit(1);
    }

    string data;
    getline(cin,data);
    const char * sendData;
    sendData = data.c_str();   //string转const char* 
                                
    /*
    send()用来将数据由指定的socket传给对方主机
    int send(int s, const void * msg, int len, unsigned int flags)
        s为已建立好连接的socket，msg指向数据内容，len则为数据长度，参数flags一般设0
        成功则返回实际传送出去的字符数，失败返回-1，错误原因存于error
    */
    send(sclient, sendData, strlen(sendData), 0);
    while(true){
        char recData[255];
        char sendData[255];
        int ret = recv(sclient, recData, 255, 0);
        if (ret>0)
        {
            recData[ret] = 0x00;
            cout << recData << endl;
        }
        else break;
        cin.getline(sendData, 255);
        send(sclient, sendData, strlen(sendData), 0);
        if(!strcmp(sendData, "byebye")) break;
    }
    close(sclient);
    system("pause");
    return 0;
}