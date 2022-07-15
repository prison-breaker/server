#include "stdafx.h"
#include "Server.h"

bool									CServer::m_InvincibleMode{};

MSG_TYPE								CServer::m_MsgType{};

vector<vector<shared_ptr<CGameObject>>> CServer::m_GameObjects{};
vector<shared_ptr<CEventTrigger>>       CServer::m_EventTriggers{};
shared_ptr<CNavMesh>                    CServer::m_NavMesh{};
vector<LIGHT>                           CServer::m_Lights{};

SERVER_TO_CLIENT_DATA                   CServer::m_SendedPacketData{};
TRIGGER_DATA							CServer::m_TriggerData{};

CServer::CServer()
{
    m_Timer = make_unique<CTimer>();

    BuildObjects();
    BuildLights();

    WSADATA WsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &WsaData))
    {
        cout << "������ �ʱ�ȭ���� ���߽��ϴ�." << endl;
        exit(1);
    }

    m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_ListenSocket == INVALID_SOCKET)
    {
        Server::ErrorQuit("socket()");
    }

    m_SocketAddress.sin_family = AF_INET;
    m_SocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    m_SocketAddress.sin_port = htons(SERVER_PORT);

    int ReturnValue{};

    ReturnValue = bind(m_ListenSocket, (SOCKADDR*)&m_SocketAddress, sizeof(m_SocketAddress));

    if (ReturnValue == SOCKET_ERROR)
    {
        Server::ErrorQuit("bind()");
    }

    ReturnValue = listen(m_ListenSocket, SOMAXCONN);

    if (ReturnValue == SOCKET_ERROR)
    {
        Server::ErrorQuit("listen()");
    }

    CreateEvents();

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
            Server::ErrorDisplay("accept()");
            continue;
        }

        if (!Server->RegisterPlayer(ClientSocket, ClientAddress))
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
    int ReturnValue{ send(ClientSocket, (char*)&ClientID, sizeof(ClientID), 0) };

    if (ReturnValue == SOCKET_ERROR)
    {
        Server::ErrorDisplay("send()");
        Server->RemovePlayer(ClientID);

        return 0;
    }

    ReturnValue = send(ClientSocket, (char*)Server->m_HasPistolGuardIndices, sizeof(Server->m_HasPistolGuardIndices), 0);

    if (ReturnValue == SOCKET_ERROR)
    {
        Server::ErrorDisplay("send()");
        Server->RemovePlayer(ClientID);

        return 0;
    }

    MSG_TYPE MsgType{};

    while (true)
    {
        WaitForSingleObject(Server->m_MainSyncEvents[0], INFINITE);

        // �޼��� ��Ŷ �����͸� �����Ѵ�.
        ReturnValue = recv(ClientSocket, (char*)&MsgType, sizeof(MsgType), MSG_WAITALL);

        if (ReturnValue == SOCKET_ERROR)
        {
            Server::ErrorDisplay("recv()");
            break;
        }
        else if (ReturnValue == 0)
        {
            break;
        }

        if (MsgType & MSG_TYPE_TITLE)
        {
            SetEvent(Server->m_ClientSyncEvents[ClientID]);
            WaitForSingleObject(Server->m_MainSyncEvents[1], INFINITE);

            bool IsReady{ Server->m_ConnectedClientCount == 2 };

            ReturnValue = send(ClientSocket, (char*)&IsReady, sizeof(IsReady), 0);

            if (ReturnValue == SOCKET_ERROR)
            {
                Server::ErrorDisplay("send()");
                break;
            }
        }
        else if (MsgType & MSG_TYPE_INGAME)
        {
            ReturnValue = recv(ClientSocket, (char*)&Server->m_ReceivedPacketData[ClientID], sizeof(Server->m_ReceivedPacketData[ClientID]), MSG_WAITALL);

            if (ReturnValue == SOCKET_ERROR)
            {
                Server::ErrorDisplay("recv()");
                break;
            }
            else if (ReturnValue == 0)
            {
                break;
            }

            if ((Server->m_ReceivedPacketData[ClientID].m_InputMask & INPUT_MASK_LMB) && (Server->m_ReceivedPacketData[ClientID].m_InputMask & INPUT_MASK_RMB))
            {
                XMFLOAT3 CameraPosition{};

                ReturnValue = recv(ClientSocket, (char*)&CameraPosition, sizeof(CameraPosition), MSG_WAITALL);

                if (ReturnValue == SOCKET_ERROR)
                {
                    Server::ErrorDisplay("recv()");
                    break;
                }
                else if (ReturnValue == 0)
                {
                    break;
                }

                shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(Server->m_GameObjects[OBJECT_TYPE_PLAYER][ClientID]) };

                Player->SetCameraPosition(CameraPosition);
            }

            SetEvent(Server->m_ClientSyncEvents[ClientID]);
            WaitForSingleObject(Server->m_MainSyncEvents[1], INFINITE);

            ReturnValue = send(ClientSocket, (char*)&Server->m_SendedPacketData, sizeof(Server->m_SendedPacketData), 0);

            if (ReturnValue == SOCKET_ERROR)
            {
                Server::ErrorDisplay("send()");
                break;
            }

            if (Server->m_SendedPacketData.m_MsgType & MSG_TYPE_TRIGGER)
            {
                ReturnValue = send(ClientSocket, (char*)&Server->m_TriggerData, sizeof(Server->m_TriggerData), 0);

                if (ReturnValue == SOCKET_ERROR)
                {
                    Server::ErrorDisplay("send()");
                    break;
                }
            }
        }
    }

    Server->RemovePlayer(ClientID);

    return 0;
}

void CServer::CreateEvents()
{
    m_MainSyncEvents[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_MainSyncEvents[1] = CreateEvent(NULL, TRUE, FALSE, NULL);

    for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
    {
        m_ClientSyncEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
}

void CServer::BuildObjects()
{
    LoadNavMeshFromFile(TEXT("Navigation/NavMesh.bin"));
    LoadSceneInfoFromFile(TEXT("Scenes/GameScene.bin"));
    LoadEventTriggerFromFile(TEXT("Triggers/EventTriggers.bin"));
}

void CServer::BuildLights()
{
    m_Lights.resize(1);

    m_Lights[0].m_IsActive = true;
    m_Lights[0].m_Position = XMFLOAT3(0.0f, 50.0f, 0.0f);
    m_Lights[0].m_SpotLightAngle = XMConvertToRadians(90.0f);
}

void CServer::LoadSceneInfoFromFile(const tstring& FileName)
{
    tstring Token{};

    shared_ptr<LOADED_MODEL_INFO> ModelInfo{};

    UINT PlayerID{};
    UINT ObjectType{};

    unordered_map<tstring, shared_ptr<CMesh>> MeshCaches{};

    LoadMeshCachesFromFile(TEXT("MeshesAndMaterials/Meshes.bin"), MeshCaches);

    // Ÿ�� ����ŭ �� ������ ũ�⸦ ���Ҵ��Ѵ�.
    m_GameObjects.resize(OBJECT_TYPE_STRUCTURE + 1);

    tifstream InFile{ FileName, ios::binary };

    while (true)
    {
        File::ReadStringFromFile(InFile, Token);

        if (Token == TEXT("<Name>"))
        {
            File::ReadStringFromFile(InFile, Token);

            ModelInfo = CGameObject::LoadObjectFromFile(Token, MeshCaches);
        }
        else if (Token == TEXT("<Type>"))
        {
            ObjectType = File::ReadIntegerFromFile(InFile);
        }
        else if (Token == TEXT("<TransformMatrix>"))
        {
            XMFLOAT4X4 TransformMatrix{};

            InFile.read(reinterpret_cast<TCHAR*>(&TransformMatrix), sizeof(XMFLOAT4X4));

            switch (ObjectType)
            {
            case OBJECT_TYPE_PLAYER:
            {
                // �÷��̾� ��ü�� �����Ѵ�.
                shared_ptr<CPlayer> Player{ make_shared<CPlayer>() };

                Player->SetID(PlayerID++);
                Player->SetChild(ModelInfo->m_Model);
                Player->SetTransformMatrix(TransformMatrix);
                Player->UpdateTransform(Matrix4x4::Identity());
                Player->SetAnimationController(ModelInfo);
                Player->Initialize();

                m_GameObjects[ObjectType].push_back(Player);
            }
            break;
            case OBJECT_TYPE_NPC:
            {
                XMFLOAT3 TargetPosition{};

                // <TargetPosition>
                File::ReadStringFromFile(InFile, Token);
                InFile.read(reinterpret_cast<TCHAR*>(&TargetPosition), sizeof(XMFLOAT3));

                // ������ ��ü�� �����Ѵ�.
                shared_ptr<CGuard> Guard{ make_shared<CGuard>() };

                Guard->SetChild(ModelInfo->m_Model);
                Guard->SetTransformMatrix(TransformMatrix);
                Guard->UpdateTransform(Matrix4x4::Identity());
                Guard->SetAnimationController(ModelInfo);
                Guard->FindPatrolNavPath(m_NavMesh, TargetPosition);
                Guard->Initialize();

                m_GameObjects[ObjectType].push_back(Guard);
            }
            break;
            case OBJECT_TYPE_TERRAIN:
            case OBJECT_TYPE_STRUCTURE:
            {
                // ���� �� ������ ��ü�� �����Ѵ�.
                shared_ptr<CGameObject> Architecture{ make_shared<CGameObject>() };

                Architecture->SetChild(ModelInfo->m_Model);
                Architecture->SetTransformMatrix(TransformMatrix);
                Architecture->UpdateTransform(Matrix4x4::Identity());
                Architecture->Initialize();

                m_GameObjects[ObjectType].push_back(Architecture);
            }
            break;
            }
        }
        else if (Token == TEXT("</GameScene>"))
        {
            for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
            {
                m_ReceivedPacketData[i].m_WorldMatrix = m_GameObjects[OBJECT_TYPE_PLAYER][i]->GetWorldMatrix();
            }

            tcout << endl;
            break;
        }
    }
}

void CServer::LoadMeshCachesFromFile(const tstring& FileName, unordered_map<tstring, shared_ptr<CMesh>>& MeshCaches)
{
    tstring Token{};

    tifstream InFile{ FileName, ios::binary };

    while (true)
    {
        File::ReadStringFromFile(InFile, Token);

        if (Token == TEXT("<Meshes>"))
        {
            UINT MeshCount{ File::ReadIntegerFromFile(InFile) };

            if (MeshCount > 0)
            {
                MeshCaches.reserve(MeshCount);
                tcout << FileName << TEXT(" �ε� ����...") << endl;
            }
        }
        else if (Token == TEXT("<Mesh>"))
        {
            shared_ptr<CMesh> Mesh{ make_shared<CMesh>() };

            Mesh->LoadMeshInfoFromFile(InFile);
            MeshCaches.emplace(Mesh->GetName(), Mesh);
        }
        else if (Token == TEXT("<SkinnedMesh>"))
        {
            shared_ptr<CSkinnedMesh> SkinnedMesh{ make_shared<CSkinnedMesh>() };

            SkinnedMesh->LoadMeshInfoFromFile(InFile);
            MeshCaches.emplace(SkinnedMesh->GetName(), SkinnedMesh);
        }
        else if (Token == TEXT("</Meshes>"))
        {
            tcout << FileName << TEXT(" �ε� �Ϸ�...(�޽� ��: ") << MeshCaches.size() << ")" << endl << endl;
            break;
        }
    }
}

void CServer::LoadNavMeshFromFile(const tstring& FileName)
{
    m_NavMesh = make_shared<CNavMesh>();
    m_NavMesh->LoadNavMeshFromFile(FileName);
}

void CServer::LoadEventTriggerFromFile(const tstring& FileName)
{
    shared_ptr<CEventTrigger> EventTrigger{};

    // ���踦 ����ϴ� Ʈ���Ÿ� �߰��Ѵ�.
    // ����� ������ �ٸ� ������(Index: 0, 1)�� �����ϰ� �ִ�.
    for (UINT i = 0; i < 2; ++i)
    {
        if (m_GameObjects[OBJECT_TYPE_NPC][i])
        {
            shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(m_GameObjects[OBJECT_TYPE_NPC][i]) };

            EventTrigger = make_shared<CGetKeyEventTrigger>();
            Guard->SetEventTrigger(EventTrigger);

            m_EventTriggers.push_back(EventTrigger);
        }
    }

    // ������ ����ϴ� Ʈ���Ÿ� �߰��Ѵ�.
    // ������ ���踦 ���� ���� ������ ������(Index: 2 ~ 14) �� 5���� �����ϰ� �ִ�.
    vector<UINT> Indices{};

    Indices.resize(m_GameObjects[OBJECT_TYPE_NPC].size() - 2);
    iota(Indices.begin(), Indices.end(), 2);
    shuffle(Indices.begin(), Indices.end(), default_random_engine{ random_device{}() });
    sort(Indices.begin(), Indices.begin() + 5);

    for (UINT i = 0; i < 5; ++i)
    {
        if (m_GameObjects[OBJECT_TYPE_NPC][Indices[i]])
        {
            shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(m_GameObjects[OBJECT_TYPE_NPC][Indices[i]]) };

            EventTrigger = make_shared<CGetPistolEventTrigger>();
            Guard->SetEventTrigger(EventTrigger);

            m_EventTriggers.push_back(EventTrigger);
            m_HasPistolGuardIndices[i] = Indices[i];
        }
    }

    tstring Token{};

    tcout << FileName << TEXT(" �ε� ����...") << endl;

    tifstream InFile{ FileName, ios::binary };

    while (true)
    {
        File::ReadStringFromFile(InFile, Token);

        if (Token == TEXT("<EventTriggers>"))
        {
            UINT TriggerCount{ File::ReadIntegerFromFile(InFile) };

            m_EventTriggers.reserve(TriggerCount);
        }
        else if (Token == TEXT("<Type>"))
        {
            TRIGGER_TYPE TriggerType{ static_cast<TRIGGER_TYPE>(File::ReadIntegerFromFile(InFile)) };

            switch (TriggerType)
            {
            case TRIGGER_TYPE_OPEN_DOOR:
                EventTrigger = make_shared<COpenDoorEventTrigger>();
                break;
            case TRIGGER_TYPE_OPEN_ELEC_PANEL:
                EventTrigger = make_shared<CPowerDownEventTrigger>();
                break;
            case TRIGGER_TYPE_SIREN:
                EventTrigger = make_shared<CSirenEventTrigger>();
                break;
            case TRIGGER_TYPE_OPEN_GATE:
                EventTrigger = make_shared<COpenGateEventTrigger>();
                break;
            }

            EventTrigger->LoadEventTriggerFromFile(InFile);
            m_EventTriggers.push_back(EventTrigger);
        }
        else if (Token == TEXT("</EventTriggers>"))
        {
            break;
        }
    }

    tcout << FileName << TEXT(" �ε� �Ϸ�...") << endl << endl;
}

UINT CServer::GetValidClientID() const
{
    for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
    {
        if (!m_ClientSocketInfos[i].m_Socket)
        {
            return i;
        }
    }

    return UINT_MAX;
}

bool CServer::RegisterPlayer(SOCKET Socket, const SOCKADDR_IN& SocketAddress)
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

    m_ConnectedClientCount += 1;

    return true;
}

void CServer::RemovePlayer(UINT ID)
{
    closesocket(m_ClientSocketInfos[ID].m_Socket);
    SetEvent(m_ClientSyncEvents[ID]);

    cout << "[Ŭ���̾�Ʈ ����] " << "IP : " << inet_ntoa(m_ClientSocketInfos[ID].m_SocketAddress.sin_addr) << ", ��Ʈ��ȣ : " << ntohs(m_ClientSocketInfos[ID].m_SocketAddress.sin_port) << endl;

    memset(&m_ClientSocketInfos[ID], NULL, sizeof(SOCKET_INFO));

    m_ConnectedClientCount -= 1;
}

void CServer::GameLoop()
{
    while (true)
    {
        ResetEvent(m_MainSyncEvents[1]);
        SetEvent(m_MainSyncEvents[0]);

        for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
        {
            if (m_ClientSocketInfos[i].m_Socket)
            {
                WaitForSingleObject(m_ClientSyncEvents[i], INFINITE);
            }
        }

        m_Timer->Tick(0.0f);

        ProcessInput();
        UpdatePlayerInfo();
        Animate(m_Timer->GetElapsedTime());
        UpdateSendedPacketData();

        ResetEvent(m_MainSyncEvents[0]);
        SetEvent(m_MainSyncEvents[1]);
    }
}

void CServer::ProcessInput()
{
    if (GetAsyncKeyState('I') & 0x0001)
    {
        if (m_InvincibleMode)
        {
            m_InvincibleMode = false;

            tcout << TEXT("������带 ��Ȱ��ȭ�մϴ�.") << endl;
        }
        else
        {
            m_InvincibleMode = true;

            tcout << TEXT("������带 Ȱ��ȭ�մϴ�.") << endl;
        }
    }
}

void CServer::UpdatePlayerInfo()
{
    memset(&m_TriggerData, NULL, sizeof(m_TriggerData));

    for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
    {
        if (m_GameObjects[OBJECT_TYPE_PLAYER][i])
        {
            shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(m_GameObjects[OBJECT_TYPE_PLAYER][i]) };

            if (Player->GetHealth() > 0)
            {
                Player->SetTransformMatrix(m_ReceivedPacketData[i].m_WorldMatrix);
                Player->UpdateTransform(Matrix4x4::Identity());
                Player->ProcessInput(m_Timer->GetElapsedTime(), m_ReceivedPacketData[i].m_InputMask);
            }
        }
    }
}

void CServer::Animate(float ElapsedTime)
{
    for (UINT i = OBJECT_TYPE_PLAYER; i <= OBJECT_TYPE_NPC; ++i)
    {
        for (const auto& GameObject : m_GameObjects[i])
        {
            if (GameObject)
            {
                GameObject->Animate(ElapsedTime);
            }
        }
    }

    for (const auto& GameObject : m_GameObjects[OBJECT_TYPE_STRUCTURE])
    {
        if (GameObject)
        {
            GameObject->Animate(ElapsedTime);
            GameObject->UpdateTransform(Matrix4x4::Identity());
        }
    }
   
    // ��ȣ�ۿ��� �Ͼ ��� Ʈ������ �۾��� �����Ѵ�.
    for (const auto& EventTrigger : m_EventTriggers)
    {
        if (EventTrigger)
        {
            EventTrigger->Update(ElapsedTime);
        }
    }

    CalculateTowerLightCollision();
}

void CServer::CalculateTowerLightCollision()
{
    if (m_Lights[0].m_IsActive)
	{ 	
		// ���� �����ǰ� ���⺤�͸� Ȱ���� ��鿡 �����ϴ� �߽����� ����Ѵ�. 
		float LightAngle{ Vector3::Angle(m_Lights[0].m_Direction, XMFLOAT3(0.0f, -1.0f, 0.0f)) };  // ������ ���� ���� ���
		float HypotenuseLength{ m_Lights[0].m_Position.y / cosf(XMConvertToRadians(LightAngle)) }; // ������ ���� ���
		float Radian{ HypotenuseLength * tanf(XMConvertToRadians(10.0f)) };                        // ������ ������� ���� ������

		// ��鿡 �����ϴ� �� ���
		XMFLOAT3 LightedPosition{ Vector3::Add(m_Lights[0].m_Position , Vector3::ScalarProduct(HypotenuseLength, m_Lights[0].m_Direction, false)) };

		for (const auto& Object : m_GameObjects[OBJECT_TYPE_PLAYER])
		{
			shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(Object) };

			if (Player)
			{
				if (Player->GetHealth() > 0)
				{
					if (Math::Distance(Player->GetPosition(), LightedPosition) < Radian)
					{
						XMFLOAT3 Direction = Vector3::Normalize(Vector3::Subtract(Player->GetPosition(), m_Lights[0].m_Position));

						float NearestHitDistance{ FLT_MAX };
						float HitDistance{};
						bool HitCheck{};

						for (const auto& GameObject : m_GameObjects[OBJECT_TYPE_STRUCTURE])
						{
							if (GameObject)
							{
								shared_ptr<CGameObject> IntersectedObject{ GameObject->PickObjectByRayIntersection(m_Lights[0].m_Position, Direction, HitDistance, HypotenuseLength) };

								if (IntersectedObject && HitDistance < HypotenuseLength)
								{
									HitCheck = true;
									break;
								}
							}
						}

						if (!HitCheck)
						{
							for (const auto& GameObject : m_GameObjects[OBJECT_TYPE_NPC])
							{
								if (GameObject)
								{
									shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(GameObject) };

									if (Guard->GetHealth() > 0)
									{
										// ��������� �浹 �� ��� �ֺ� ������ �ִ� �������� �÷��̾ �i�� �����Ѵ�.
										if (Math::Distance(LightedPosition, Guard->GetPosition()) <= 150.0f)
										{
											if (Guard->GetStateMachine()->IsInState(CGuardIdleState::GetInstance()) ||
												Guard->GetStateMachine()->IsInState(CGuardPatrolState::GetInstance()) ||
												Guard->GetStateMachine()->IsInState(CGuardReturnState::GetInstance()))
											{
												Guard->FindNavPath(m_NavMesh, Player->GetPosition(), m_GameObjects);
												Guard->GetStateMachine()->ChangeState(CGuardAssembleState::GetInstance());
											}
										}
									}
								}
							}

							return;
						}
					}
				}
			}
		}

        m_Lights[0].m_SpotLightAngle += m_Timer->GetElapsedTime();
		m_Lights[0].m_Direction = Vector3::Normalize(XMFLOAT3(cosf(m_Lights[0].m_SpotLightAngle), -1.0f, sinf(m_Lights[0].m_SpotLightAngle)));
	}
}

void CServer::UpdateSendedPacketData()
{
    m_SendedPacketData.m_MsgType = (m_ConnectedClientCount == 2) ? m_MsgType : MSG_TYPE_DISCONNECTION;
    m_MsgType = MSG_TYPE_NONE;

    for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
    {
        m_SendedPacketData.m_PlayerWorldMatrices[i] = m_GameObjects[OBJECT_TYPE_PLAYER][i]->GetWorldMatrix();
        m_SendedPacketData.m_PlayerAnimationClipTypes[i] = m_GameObjects[OBJECT_TYPE_PLAYER][i]->GetAnimationController()->GetAnimationClipType();
    }

    for (UINT i = 0; i < MAX_NPC_COUNT; ++i)
    {
        m_SendedPacketData.m_NPCWorldMatrices[i] = m_GameObjects[OBJECT_TYPE_NPC][i]->GetWorldMatrix();
        m_SendedPacketData.m_NPCAnimationClipTypes[i] = m_GameObjects[OBJECT_TYPE_NPC][i]->GetAnimationController()->GetAnimationClipType();
    }

    m_SendedPacketData.m_TowerLightDirection = m_Lights[0].m_Direction;
}
