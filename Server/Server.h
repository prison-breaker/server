#pragma once
#include "Timer.h"
#include "Player.h"
#include "Guard.h"
#include "EventTriggers.h"

class CServer
{
public:
	static vector<vector<shared_ptr<CGameObject>>> m_GameObjects;
	static vector<shared_ptr<CEventTrigger>>	   m_EventTriggers;
	static shared_ptr<CNavMesh>				       m_NavMesh;
	static vector<LIGHT>						   m_Lights;

private:
	bool		                                   m_GameStart{};
											       
	// Ŭ���̾�Ʈ�� �����ϱ� ���� ��� ����	          
	SOCKET                                         m_ListenSocket{};
											       
	// ������ ���� �ּ� ����ü				       
	SOCKADDR_IN                                    m_SocketAddress{};

	// ������ Ŭ���̾�Ʈ���� ���� ������ ��� �ִ� �迭
	SOCKET_INFO                                    m_ClientSocketInfos[MAX_CLIENT_CAPACITY]{};

	HANDLE										   m_MainSyncEvents[2]{};
	HANDLE										   m_ClientSyncEvents[MAX_CLIENT_CAPACITY]{};

	// ���� �ֱٿ� ������ Ŭ���̾�Ʈ�� �ε���  
	UINT									       m_RecentClientID{};

	unique_ptr<CTimer>							   m_Timer{};
	
	CLIENT_TO_SERVER_DATA						   m_ReceivedPacketData[MAX_CLIENT_CAPACITY]{};
	SERVER_TO_CLIENT_DATA                          m_SendedPacketData{};

public:
	CServer();
	~CServer();

	static DWORD WINAPI AcceptClient(LPVOID Arg);
	static DWORD WINAPI ProcessClient(LPVOID Arg);

	void CreateEvents();

	void BuildObjects();
	void BuildLights();

	void LoadSceneInfoFromFile(const tstring& FileName);
	void LoadMeshCachesFromFile(const tstring& FileName, unordered_map<tstring, shared_ptr<CMesh>>& MeshCaches);
	void LoadNavMeshFromFile(const tstring& FileName);
	void LoadEventTriggerFromFile(const tstring& FileName);

	UINT GetValidClientID() const;

	bool RegisterPlayer(SOCKET Socket, const SOCKADDR_IN& SocketAddress);
	void RemovePlayer(UINT ID);

	void GameLoop();

	void UpdatePlayerInfo();
	void CalculateTowerLightCollision();

	void UpdateSendedPacketData();
};
