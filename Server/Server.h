#pragma once

class CServer
{
public:
	CServer();
	~CServer();

private:
	// 클라이언트를 수용하기 위한 대기 소켓
	SOCKET      m_ListenSocket{};

	// 서버의 소켓 주소 구조체
	SOCKADDR_IN m_SocketAddress{};

	// 접속한 클라이언트들의 소켓 정보를 담고 있는 배열
	SOCKET_INFO m_ClientSocketInfos[MAX_CLIENT_CAPACITY]{};

	// 가장 최근에 접속한 클라이언트의 인덱스
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
