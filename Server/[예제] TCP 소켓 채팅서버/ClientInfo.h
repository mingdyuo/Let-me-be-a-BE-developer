#pragma once
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <cstdio>
#define MAX_SOCKBUF 1024	//��Ŷ ũ��

enum IOOperation
{
	RECV,
	SEND
};

//WSAOVERLAPPED����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ �� �־���.
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;		//Overlapped I/O����ü
	SOCKET		m_socketClient;			//Ŭ���̾�Ʈ ����
	WSABUF		m_wsaBuf;				//Overlapped I/O�۾� ����
	IOOperation m_eOperation;			//�۾� ���� ����
};

//Ŭ���̾�Ʈ ������ ������� ����ü
struct stClientInfo
{
	INT32 mIndex;
	SOCKET			m_socketClient;			//Cliet�� ����Ǵ� ����
	stOverlappedEx	m_stRecvOverlappedEx;	//RECV Overlapped I/O�۾��� ���� ����
	stOverlappedEx	m_stSendOverlappedEx;	//SEND Overlapped I/O�۾��� ���� ����
	
	char			mRecvBuf[MAX_SOCKBUF]; //������ ����
	char			mSendBuf[MAX_SOCKBUF]; //������ ����

	stClientInfo(): mIndex(0), m_socketClient(INVALID_SOCKET)
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
	}

	stClientInfo(UINT32 index): mIndex(mIndex), m_socketClient(INVALID_SOCKET)
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
	}

public:
	INT32 GetIndex(){return mIndex;}
	SOCKET GetSock() {return m_socketClient; }
	bool IsConnected() {return m_socketClient != INVALID_SOCKET;}

	void SendCompleted(const UINT32 dataSize_){
		printf("[�۽� �Ϸ�] bytes : %d\n", dataSize_);
	}

	void Close(bool bIsForce = false){
		struct linger stLinger = {0, 0};
		if(bIsForce)
			stLinger.l_onoff = 1;
		
		shutdown(m_socketClient, SD_BOTH);

		setsockopt(m_socketClient, SOL_SOCKET, SO_LINGER, (char*)&stLinger, sizeof(stLinger));

		closesocket(m_socketClient);
		m_socketClient = INVALID_SOCKET;
	}

	bool BindIOCompletionPort(HANDLE iocpHandle_){
		HANDLE hIOCP = CreateIoCompletionPort(
			(HANDLE)GetSock(),
			iocpHandle_,
			(ULONG_PTR)(this),
			0
		);
		if(hIOCP == INVALID_HANDLE_VALUE){
			printf("[����] CreateIoCompletionPort()�Լ� ����: %d\n", GetLastError());
			return false;
		}

		return true;
	}

	bool OnConnect(HANDLE iocpHandle_, SOCKET socket_){
		m_socketClient = socket_;
		if(BindIOCompletionPort(iocpHandle_) == false)
			return false;

		return BindRecv(); 
	}

	bool BindRecv(){
		DWORD dwFlag = 0;
		DWORD dwRecvNumBytes = 0;

		m_stRecvOverlappedEx.m_wsaBuf.len = MAX_SOCKBUF;
		m_stRecvOverlappedEx.m_wsaBuf.buf = mRecvBuf;
		m_stRecvOverlappedEx.m_eOperation = RECV;

		int nRet = WSARecv(
			m_socketClient,
			&(m_stRecvOverlappedEx.m_wsaBuf),
			1,
			&dwRecvNumBytes,
			&dwFlag,
			(LPWSAOVERLAPPED) & (m_stRecvOverlappedEx),
			NULL
		);

		if(nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING)){
			printf("[����] WSARecv()�Լ� ���� : %d\n", WSAGetLastError());
			return false;
		}
		return true;
	}

	// 1���� ������ ������ ȣ���ؾ���
	bool SendMsg(const UINT32 dataSize_, char* pMsg_){
		stOverlappedEx* sendOverlappedEx = new stOverlappedEx;

	}



};
