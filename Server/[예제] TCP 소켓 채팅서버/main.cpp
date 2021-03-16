#include "IOCPServer.h"
#include <string>
#include <iostream>

const UINT16 SERVER_PORT = 9898;
const UINT16 MAX_CLIENT = 100;		//�� �����Ҽ� �ִ� Ŭ���̾�Ʈ ��

int main()
{
	IOCPServer iocpServer;

	iocpServer.InitSocket();
	iocpServer.BindAndListen(SERVER_PORT);
	iocpServer.StartServer(MAX_CLIENT);

	printf("�ƹ� Ű�� ���� ������ ����մϴ�\n");
	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);

		if (inputCmd == "quit")
		{
			break;
		}
	}

	iocpServer.DestroyThread();
	return 0;
}

