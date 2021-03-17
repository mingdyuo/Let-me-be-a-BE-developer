#include "IOCPClient.h"
#define MAX_SOCKBUF 1024

void ErrorHandling(char *message);
static const int SERVER_PORT = 9898;

 
int main()
{
    IOCPClient iocpClient;
    HANDLE sender, recver;
    iocpClient.InitSocket();
    iocpClient.ConnectServer(SERVER_PORT);
    iocpClient.SetNickname();
    iocpClient.CreateThreads(&sender, &recver);

    system("cls");
    printf("[%s��] ��ȭ�濡 �����ϼ̽��ϴ�.\n", iocpClient.getNickname());
    printf("[�˸�] quit�� �Է½� Ŭ���̾�Ʈ �����մϴ�.\n");
    printf("�λ縻�� �ǳ׺����� !! :");


    WaitForSingleObject (sender, INFINITE);
    WaitForSingleObject (recver, INFINITE);
    
    iocpClient.close();

    printf("[�˸�] Ŭ���̾�Ʈ�� ����Ǿ����ϴ�. �ƹ�Ű�� ������ â�� �����մϴ�.\n");
    getchar();
    return 0;
}
 
