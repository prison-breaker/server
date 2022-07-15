#pragma once
#include "Timer.h"
#include "Player.h"
#include "Guard.h"
#include "EventTriggers.h"

class CServer
{
public:
	static bool									   m_InvincibleMode;

	static MSG_TYPE								   m_MsgType;

	static vector<vector<shared_ptr<CGameObject>>> m_GameObjects;
	static vector<shared_ptr<CEventTrigger>>	   m_EventTriggers;
	static shared_ptr<CNavMesh>				       m_NavMesh;
	static vector<LIGHT>						   m_Lights;

	static SERVER_TO_CLIENT_DATA                   m_SendedPacketData;
	static TRIGGER_DATA							   m_TriggerData;

private:				              
	SOCKET                                         m_ListenSocket{};                           // Ŭ���̾�Ʈ�� �����ϱ� ���� ��� ����
	SOCKADDR_IN                                    m_SocketAddress{};                          // ������ ���� �ּ� ����ü
	SOCKET_INFO                                    m_ClientSocketInfos[MAX_PLAYER_CAPACITY]{}; // ������ Ŭ���̾�Ʈ���� ���� ������ ��� �ִ� �迭

	HANDLE										   m_MainSyncEvents[2]{};
	HANDLE										   m_ClientSyncEvents[MAX_PLAYER_CAPACITY]{};

	UINT										   m_ConnectedClientCount{};
	UINT									       m_RecentClientID{};

	UINT								           m_HasPistolGuardIndices[5]{};

	CLIENT_TO_SERVER_DATA						   m_ReceivedPacketData[MAX_PLAYER_CAPACITY]{};

	unique_ptr<CTimer>							   m_Timer{};

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

	void ProcessInput();

	void UpdatePlayerInfo();
	void Animate(float ElapsedTime);
	void CalculateTowerLightCollision();

	void UpdateSendedPacketData();
};
