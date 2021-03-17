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

enum ClientAction{
	ENTER = 10,
	EXIT = 20,
	CHAT = 30
};

//WSAOVERLAPPED����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ �� �־���.
struct stOverlappedEx
{
	WSAOVERLAPPED m_wsaOverlapped;		//Overlapped I/O����ü
	SOCKET		m_socketClient;			//Ŭ���̾�Ʈ ����
	WSABUF		m_wsaBuf;				//Overlapped I/O�۾� ����
	IOOperation m_eOperation;			//�۾� ���� ����
};

struct stClientInfo
{
	size_t mIndex;
	SOCKET			m_socketClient;			
	stOverlappedEx	m_stRecvOverlappedEx;	
	stOverlappedEx	m_stSendOverlappedEx;	
	bool			mbHasNick;

	char 			mNickname[32];
	char			mRecvBuf[MAX_SOCKBUF]; 
	char			mSendBuf[MAX_SOCKBUF]; 

	stClientInfo(): mIndex(0), m_socketClient(INVALID_SOCKET), mbHasNick(false)
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
	}

	stClientInfo(UINT32 index): mIndex(mIndex), m_socketClient(INVALID_SOCKET), mbHasNick(false)
	{
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
	}

public:
	size_t GetIndex(){return mIndex;}
	SOCKET GetSock() {return m_socketClient; }
	bool IsConnected() {return m_socketClient != INVALID_SOCKET;}

	void SendCompleted(const UINT32 dataSize_){
		printf("[�۽� �Ϸ�] bytes : %d\n", dataSize_);
	}

	void setNickname(){
		strcpy(mNickname, mRecvBuf);
	}

	void Initialize(){
		Close();
		ZeroMemory(&m_stRecvOverlappedEx, sizeof(stOverlappedEx));
		ZeroMemory(&m_stSendOverlappedEx, sizeof(stOverlappedEx));
		memset(mNickname, 0, sizeof(mNickname));
		mIndex = 0;
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
		ZeroMemory(sendOverlappedEx, sizeof(stOverlappedEx));
		sendOverlappedEx->m_wsaBuf.len = dataSize_;
		sendOverlappedEx->m_wsaBuf.buf = new char[dataSize_];
		CopyMemory(sendOverlappedEx->m_wsaBuf.buf, pMsg_, dataSize_);
		sendOverlappedEx->m_eOperation = SEND;
		
		DWORD dwRecvNumBytes = 0;
		int nRet = WSASend(m_socketClient,
			&(sendOverlappedEx->m_wsaBuf),
			1,
			&dwRecvNumBytes,
			0,
			(LPWSAOVERLAPPED)sendOverlappedEx,
			NULL);

		//socket_error�̸� client socket�� �������ɷ� ó���Ѵ�.
		if (nRet == SOCKET_ERROR && (WSAGetLastError() != ERROR_IO_PENDING))
		{
			printf("[�˸�] Ŭ���̾�Ʈ ���� ���� : %d\n", WSAGetLastError());
			Initialize();
			return false;
		}

		return true;
	}



};

