#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")  // ���� ���̺귯���� "Ws2_32.lib"�� ���Խ�Ű��� ��Ŀ���� �˷��ش�.

u_short constexpr PORT = 27015;
size_t constexpr DEFAULT_BUF_LEN = 1024;

enum class eClientStatus
{
    Normal,
    Closed,
    Error,
};

void HandleError(const char* msg);
void InitializeWinSock();
SOCKET CreateSocket();
SOCKADDR_IN SetServSocketAddr();
void BindSocket(SOCKET* sock, SOCKADDR_IN* addr);
void ListenSocket(SOCKET* sock);
SOCKET AcceptClient(SOCKET* sock);
eClientStatus HandleClient(SOCKET* client, char* buffer);
void ShutdownSocket(SOCKET* client, int option);

int main()
{
    InitializeWinSock();  // WS2_32 ���̺귯�� �ʱ�ȭ

    SOCKET servSock = CreateSocket();  //  ���� ���� ���� 
    SOCKADDR_IN servAddr = SetServSocketAddr();  // ���� ���Ͽ� �Ҵ��� ���� ���� ����ü ����
    BindSocket(&servSock, &servAddr);  // ������ ���ε� ���·� �����
    ListenSocket(&servSock);  // ������ ������ ���·� ����� Ŭ���̾�Ʈ�� ��ٸ��� 

    // �ټ� ���� Ŭ���̾�Ʈ ��û�� ���� �� �����Ѵ�. 
    for (int i = 0; i < 5; i++)
    {
        // Ŭ���̾�Ʈ�� ���� ��û�� ������ �����ϰ� Ŭ���̾�Ʈ�� ����� ������ �����Ѵ�. 
        SOCKET clientSock = AcceptClient(&servSock);

        char recvBuf[DEFAULT_BUF_LEN];  // ������ �޼����� �־���� ����
        bool clientClosed = false;

        while (clientClosed == false)  // Ŭ���̾�Ʈ�� ������ ���� ������ �޼����� �����Ѵ� 
        {
            eClientStatus status = HandleClient(&clientSock, recvBuf);

            switch (status)
            {
            case eClientStatus::Closed:
                std::cout << "-----Client Closed----" << std::endl;
                clientClosed = true;
                break;
            case eClientStatus::Error:
                std::cout << "Error on recv()" << std::endl;
                clientClosed = true;
                break;
            default:  // eClientStatus::Normal
                break;
            }
        }

        ShutdownSocket(&clientSock, SD_SEND);  // ���� ���� (������ �����ϵ��� ������´�)
        closesocket(clientSock);  // Ŭ���̾�Ʈ ���� ����
    }

    closesocket(servSock);  // ���� ���� ����
    WSACleanup();  // WS2_32 ���̺귯�� ���� 
}

void HandleError(const char* fname)
{
    std::cout << "Error on " << fname << std::endl;
    WSACleanup();
    exit(1);
}

void InitializeWinSock()
{
    WSADATA WSAData; // windows socket ������ ���� socket�� ���� ���� ���� ��� �ִ� ����ü

    int result = WSAStartup(MAKEWORD(2, 2), &WSAData);  // WS2_32 ���̺귯�� �ʱ�ȭ
    if (result != 0)
    {
        std::cout << "Error on Initializing WS2_32.dll" << std::endl;
        exit(1);
    }
}

SOCKET CreateSocket()
{
    SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (sock == INVALID_SOCKET)
    {
        HandleError("socket()");
    }
    return sock;
}

SOCKADDR_IN SetServSocketAddr()
{
    SOCKADDR_IN addr = SOCKADDR_IN();  // SOCKADDR_IN ����ü �ʱ�ȭ

    addr.sin_family = AF_INET;  // IPv4 ü��
    // ��Ʋ ����� ����� ���� IP �ּ� ������ -> �� ����� ����� ������  
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);

    return addr;
}

void BindSocket(SOCKET* sock, SOCKADDR_IN* addr)
{
    int result = bind(*sock, (sockaddr*)addr, sizeof(*addr));
    if (result == SOCKET_ERROR)
    {
        HandleError("bind()");
    }
}

void ListenSocket(SOCKET* sock)
{
    int result = listen(*sock, SOMAXCONN);  // ���ϰ� �ִ� ���� ��⿭ ũ�� ����
    if (result == SOCKET_ERROR)
    {
        HandleError("listen()");
    }
}

SOCKET AcceptClient(SOCKET* sock)
{
    SOCKADDR_IN clientAddr = SOCKADDR_IN();  // Ŭ���̾�Ʈ ���� ������ ���� ����ü ���� 
    int clientAddrSize = sizeof(clientAddr);
    SOCKET clientSock = accept(*sock, (sockaddr*)&clientAddr, &clientAddrSize);

    if (clientSock == INVALID_SOCKET)
    {
        HandleError("accept()");
    }

    std::cout << "----Client Entered----" << std::endl;

    return clientSock;
}

eClientStatus HandleClient(SOCKET* client, char* buffer)
{
    // Ŭ���̾�Ʈ�κ��� �޼��� �����ϸ� ���ۿ� �־���´� 
    int recvResult = recv(*client, buffer, DEFAULT_BUF_LEN, 0);

    if (recvResult > 0)  // ���������� ������ ���
    {
        // Ŭ���̾�Ʈ�κ��� ���� �޼����� �״�� �����ش� 
        int sendResult = send(*client, buffer, recvResult, 0);
        if (sendResult == SOCKET_ERROR)
        {
            closesocket(*client);
            HandleError("send()");
        }

        buffer[recvResult] = 0;  // ���ڿ� ���� �׻� NULL������ ä���ش�.
        std::cout << "From client: " << buffer << std::endl;

        return eClientStatus::Normal;
    }
    else if (recvResult == 0)  // Ŭ���̾�Ʈ�� ������ ������ ��� 
    {
        return eClientStatus::Closed;
    }
    else  // ������ �� ��� 
    {
        return eClientStatus::Error;
    }
}

void ShutdownSocket(SOCKET* client, int option)
{
    int shutDownResult = shutdown(*client, option);

    if (shutDownResult == SOCKET_ERROR)
    {
        closesocket(*client);
        HandleError("shutdown()");
    }
}
