#include "stdafx.h"
#include "Server.h"

CServer::CServer()
{
    WSADATA WsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &WsaData))
    {
        cout << "������ �ʱ�ȭ���� ���߽��ϴ�." << endl;
        exit(1);
    }

    m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_ListenSocket == INVALID_SOCKET)
    {
        ErrorQuit("socket()");
    }

    m_SocketAddress.sin_family = AF_INET;
    m_SocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    m_SocketAddress.sin_port = htons(SERVER_PORT);

    int ReturnValue{};

    ReturnValue = bind(m_ListenSocket, (SOCKADDR*)&m_SocketAddress, sizeof(m_SocketAddress));

    if (ReturnValue == SOCKET_ERROR)
    {
        ErrorQuit("bind()");
    }

    ReturnValue = listen(m_ListenSocket, SOMAXCONN);

    if (ReturnValue == SOCKET_ERROR)
    {
        ErrorQuit("listen()");
    }

    HANDLE hThread{ CreateThread(NULL, 0, AcceptClient, (LPVOID)this, 0, NULL) };

    if (hThread)
    {
        CloseHandle(hThread);
    }
}

CServer::~CServer()
{
    closesocket(m_ListenSocket);
    WSACleanup();
}

void CServer::ErrorQuit(const char* Msg)
{
    LPVOID MsgBuffer{};

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&MsgBuffer, 0, NULL);
    MessageBox(NULL, (LPCTSTR)MsgBuffer, (LPCTSTR)Msg, MB_ICONERROR);

    LocalFree(MsgBuffer);
    exit(1);
}

void CServer::ErrorDisplay(const char* Msg)
{
    LPVOID MsgBuffer{};

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&MsgBuffer, 0, NULL);
    cout << "[" << Msg << "] " << (char*)MsgBuffer;

    LocalFree(MsgBuffer);
}

DWORD WINAPI CServer::AcceptClient(LPVOID Arg)
{
    // ���� �ڱ� �ڽſ� ���� �����ͷ�, ������ �Լ� ������ ��������� �����ϱ� ���ؼ� �̿� ���� �����Ͽ���.
    CServer* Server{ (CServer*)Arg };
    SOCKET ClientSocket{};
    SOCKADDR_IN ClientAddress{};

    while (true)
    {
        int AddressLength{ sizeof(ClientAddress) };

        ClientSocket = accept(Server->m_ListenSocket, (SOCKADDR*)&ClientAddress, &AddressLength);

        if (ClientSocket == INVALID_SOCKET)
        {
            ErrorDisplay("accept()");
            continue;
        }

        if (!Server->CreatePlayer(ClientSocket, ClientAddress))
        {
            closesocket(ClientSocket);
            continue;
        }

        cout << "[Ŭ���̾�Ʈ ����] " << "IP : " << inet_ntoa(ClientAddress.sin_addr) << ", ��Ʈ��ȣ : " << ntohs(ClientAddress.sin_port) << endl;

        HANDLE ThreadHandle{ CreateThread(NULL, 0, ProcessClient, (LPVOID)Server, 0, NULL) };

        if (ThreadHandle)
        {
            CloseHandle(ThreadHandle);
        }
        else
        {
            closesocket(ClientSocket);
        }
    }

    return 0;
}

DWORD WINAPI CServer::ProcessClient(LPVOID Arg)
{
    // ���� �ڱ� �ڽſ� ���� �����ͷ�, ������ �Լ� ������ ��������� �����ϱ� ���ؼ� �̿� ���� �����Ͽ���.
    CServer* Server{ (CServer*)Arg };
    UINT ClientID{ Server->m_RecentClientID };
    SOCKET ClientSocket{ Server->m_ClientSocketInfos[ClientID].m_Socket };

    // ���ʷ� Ŭ���̾�Ʈ���� �ʱ�ȭ�� �÷��̾��� ���̵� ������.
    int ReturnValue{ send(ClientSocket, (char*)&ClientID, sizeof(UINT), 0) };

    if (ReturnValue == SOCKET_ERROR)
    {
        ErrorDisplay("send()");
    }
    else
    {
        UINT PacketSize{};

        while (true)
        {
            // ��Ŷ �������� ũ�⸦ �����Ѵ�.
            ReturnValue = recv(ClientSocket, (char*)&PacketSize, sizeof(UINT), MSG_WAITALL);

            if (ReturnValue == SOCKET_ERROR)
            {
                ErrorDisplay("send()");
                break;
            }
            else if (ReturnValue == 0)
            {
                break;
            }

            // ��Ŷ �������� ũ�⸸ŭ ��Ŷ �����͸� �����Ѵ�.
            ReturnValue = recv(ClientSocket, (char*)&PacketSize, sizeof(UINT), MSG_WAITALL);
        }
    }
    
    Server->DestroyPlayer(ClientID);

    return 0;
}

UINT CServer::GetValidClientID() const
{
    for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i)
    {
        if (!m_ClientSocketInfos[i].m_Socket)
        {
            return i;
        }
    }

    return UINT_MAX;
}

bool CServer::CreatePlayer(SOCKET Socket, const SOCKADDR_IN& SocketAddress)
{
    UINT ValidID{ GetValidClientID() };

    if (ValidID == UINT_MAX)
    {
        return false;
    }

    m_RecentClientID = ValidID;

    m_ClientSocketInfos[ValidID].m_ID = ValidID;
    m_ClientSocketInfos[ValidID].m_Socket = Socket;
    m_ClientSocketInfos[ValidID].m_SocketAddress = SocketAddress;

    return true;
}

void CServer::DestroyPlayer(UINT ID)
{
    closesocket(m_ClientSocketInfos[ID].m_Socket);

    m_ClientSocketInfos[ID].m_Socket = NULL;

    cout << "[Ŭ���̾�Ʈ ����] " << "IP : " << inet_ntoa(m_ClientSocketInfos[ID].m_SocketAddress.sin_addr) << ", ��Ʈ��ȣ : " << ntohs(m_ClientSocketInfos[ID].m_SocketAddress.sin_port) << endl;
}
