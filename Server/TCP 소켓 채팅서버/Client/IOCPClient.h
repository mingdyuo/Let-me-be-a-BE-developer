#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <string>
#include <cstdio>
#include <iostream>
#include "Position.h"

class IOCPClient{
private:
    SOCKET mSocket;
    bool mbIsWorkerRun;
    char mNickname[32];

    Position pos;

public:
    IOCPClient():mSocket(INVALID_SOCKET), mbIsWorkerRun(true) {}

    bool OnConnect(){return mbIsWorkerRun;}
    char* getNickname(){return mNickname;}

    bool InitSocket(){
        WSADATA wsaData;
        if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0){
            printf("[����] WSAStartup()�Լ� ���� : %d\n", WSAGetLastError());
            return false;
        }

        mSocket = socket(PF_INET, SOCK_STREAM, 0); 

        if (mSocket == INVALID_SOCKET){
            printf("[����] socket()�Լ� ���� : %d\n", WSAGetLastError());
            return false;
        }
            
        printf("[����] ���� �ʱ�ȭ �Ϸ�\n");
        return true;
    }

    bool ConnectServer(int bBindPort){
        SOCKADDR_IN serverAddress;

        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); 
        serverAddress.sin_port = htons(bBindPort); 

        if (SOCKET_ERROR == connect(mSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress))){
            printf("[����] connect()�Լ� ���� : %d\n", WSAGetLastError());
            return false;
        }

        printf("[����] connect �Ϸ�\n");
        return true;
    }

    bool SetNickname(){
        do {
            printf("[�����] ��ȭ�� ����� �г����� �Է��ϼ���(�ִ� 31����)\n");
            scanf("%s", mNickname);
        } while(strlen(mNickname)==0);

        send(mSocket, mNickname, (int)strlen(mNickname), 0);
        return true;
    }

    bool CreateThreads(HANDLE* sender, HANDLE* recver){
        *recver = CreateThread(NULL, 0, StaticRecvThread, this, 0, 0);
        *sender = CreateThread(NULL, 0, StaticSendThread, this, 0, 0);
        printf("[����] ������ ���� �Ϸ�\n");

        Sleep(200);
        return true;
    }

    bool close(){
        mbIsWorkerRun = false;
        closesocket(mSocket);
        WSACleanup(); 
        return true;
    }

    static DWORD WINAPI StaticRecvThread(LPVOID arg){
        IOCPClient* This = (IOCPClient*)arg;
        return This->RecvThread();
    }

    static DWORD WINAPI StaticSendThread(LPVOID arg){
        IOCPClient* This = (IOCPClient*)arg;
        return This->SendThread();
    }

    DWORD RecvThread(){
        char received[1024];
        while (mbIsWorkerRun) { 
            memset(&received, 0, sizeof(received));
            int length = recv(mSocket, received, sizeof(received), 0);
            if(length <= 0) {
                printf("������ ���������ϴ�.\n");
                break;
            }
            received[length] = '\0';
            
            pos.Receive("����", received);
            pos.SendBox(mNickname);
        }
        return 0;
    }

    DWORD SendThread(){
        std::string msg;
        while (mbIsWorkerRun) { 
            do{
                pos.SendBox(mNickname);
                getline(std::cin, msg);
            } while(msg.empty());

            int bSuccess = send(mSocket, msg.c_str(), (int)msg.length(), 0);
            if(bSuccess == SOCKET_ERROR) break;
            pos.Receive(mNickname, msg.c_str());
            
            if (msg == "quit") {
                mbIsWorkerRun = false;
                printf("��ȭ�� �����մϴ�.\n");
                break;
            }
        }
        return 0;
    }
   
};