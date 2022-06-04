#pragma once

class CServer
{
public:
	CServer();
	~CServer();

private:
	// Ŭ���̾�Ʈ�� �����ϱ� ���� ��� ����
	SOCKET      m_ListenSocket{};

	// ������ ���� �ּ� ����ü
	SOCKADDR_IN m_SocketAddress{};

	// ������ Ŭ���̾�Ʈ���� ���� ������ ��� �ִ� �迭
	SOCKET_INFO m_ClientSocketInfos[MAX_CLIENT_CAPACITY]{};

	// ���� �ֱٿ� ������ Ŭ���̾�Ʈ�� �ε���
	UINT        m_RecentClientID{};

	CLIENT_TO_SERVER_DATA a{};

public:
	static void ErrorQuit(const char* Msg);
	static void ErrorDisplay(const char* Msg);

	static DWORD WINAPI AcceptClient(LPVOID Arg);
	static DWORD WINAPI ProcessClient(LPVOID Arg);

	UINT GetValidClientID() const;
	bool CreatePlayer(SOCKET Socket, const SOCKADDR_IN& SocketAddress);
	void DestroyPlayer(UINT ID);
};
