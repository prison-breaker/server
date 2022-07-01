#include "stdafx.h"
#include "Server.h"

MSG_TYPE								CServer::m_MsgType{};
MSG_TYPE								CServer::m_CompletedTriggers{};

vector<vector<shared_ptr<CGameObject>>> CServer::m_GameObjects{};
vector<shared_ptr<CEventTrigger>>       CServer::m_EventTriggers{};
shared_ptr<CNavMesh>                    CServer::m_NavMesh{};
vector<LIGHT>                           CServer::m_Lights{};

SERVER_TO_CLIENT_DATA                   CServer::m_SendedPacketData{};

CServer::CServer()
{
    m_Timer = make_unique<CTimer>();

    BuildObjects();
    BuildLights();

    WSADATA WsaData{};

    if (WSAStartup(MAKEWORD(2, 2), &WsaData))
    {
        cout << "윈속을 초기화하지 못했습니다." << endl;
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
    // 서버 자기 자신에 대한 포인터로, 스레드 함수 내에서 멤버변수에 접근하기 위해서 이와 같이 구현하였다.
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

        cout << "[클라이언트 접속] " << "IP : " << inet_ntoa(ClientAddress.sin_addr) << ", 포트번호 : " << ntohs(ClientAddress.sin_port) << endl;

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
    // 서버 자기 자신에 대한 포인터로, 스레드 함수 내에서 멤버변수에 접근하기 위해서 이와 같이 구현하였다.
    CServer* Server{ (CServer*)Arg };
    UINT ClientID{ Server->m_RecentClientID };
    SOCKET ClientSocket{ Server->m_ClientSocketInfos[ClientID].m_Socket };

    // 최초로 클라이언트에게 초기화된 플레이어의 아이디를 보낸다.
    int ReturnValue{ send(ClientSocket, (char*)&ClientID, sizeof(UINT), 0) };
    MSG_TYPE MsgType{};

    if (ReturnValue == SOCKET_ERROR)
    {
        Server::ErrorDisplay("send()");
    }
    else
    {
        while (true)
        {
            WaitForSingleObject(Server->m_MainSyncEvents[0], INFINITE);

            // 메세지 패킷 데이터를 수신한다.
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

                SetEvent(Server->m_ClientSyncEvents[ClientID]);
                WaitForSingleObject(Server->m_MainSyncEvents[1], INFINITE);

                ReturnValue = send(ClientSocket, (char*)&Server->m_SendedPacketData, sizeof(Server->m_SendedPacketData), 0);

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

    for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i)
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
    LIGHT Light{};

    Light.m_IsActive = true;
    Light.m_Position = XMFLOAT3(0.0f, 50.0f, 0.0f);
    Light.m_SpotLightAngle = XMConvertToRadians(90.0f);
    
    m_Lights.push_back(Light);
}

void CServer::LoadSceneInfoFromFile(const tstring& FileName)
{
    tstring Token{};

    shared_ptr<LOADED_MODEL_INFO> ModelInfo{};
    UINT ObjectType{};

    unordered_map<tstring, shared_ptr<CMesh>> MeshCaches{};

    LoadMeshCachesFromFile(TEXT("MeshesAndMaterials/Meshes.bin"), MeshCaches);

    // 타입 수만큼 각 벡터의 크기를 재할당한다.
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
                // 플레이어 객체를 생성한다.
                shared_ptr<CPlayer> Player{ make_shared<CPlayer>() };

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

                // 교도관 객체를 생성한다.
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
                // 지형 및 구조물 객체를 생성한다.
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
            for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i)
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
                tcout << FileName << TEXT(" 로드 시작...") << endl;
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
            tcout << FileName << TEXT(" 로드 완료...(메쉬 수: ") << MeshCaches.size() << ")" << endl << endl;
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
    tstring Token{};
    shared_ptr<CEventTrigger> EventTrigger{};

    tcout << FileName << TEXT(" 로드 시작...") << endl;

    tifstream InFile{ FileName, ios::binary };

    while (true)
    {
        File::ReadStringFromFile(InFile, Token);

        if (Token == TEXT("<EventTriggers>"))
        {
            UINT TriggerCount{ File::ReadIntegerFromFile(InFile) };

            m_EventTriggers.reserve(TriggerCount);
        }
        else if (Token == TEXT("<EventTrigger>"))
        {
            // <Type>
            File::ReadStringFromFile(InFile, Token);

            UINT TriggerType{ File::ReadIntegerFromFile(InFile) };

            switch (TriggerType)
            {
            case 0:
                EventTrigger = make_shared<COpenDoorEventTrigger>();
                break;
            case 1:
                EventTrigger = make_shared<CPowerDownEventTrigger>();
                break;
            case 2:
                EventTrigger = make_shared<CSirenEventTrigger>();
                break;
            case 3:
                EventTrigger = make_shared<COpenGateEventTrigger>();
                break;
            }

            EventTrigger->LoadEventTriggerFromFile(InFile);

            // <TargetRootIndex>
            File::ReadStringFromFile(InFile, Token);

            UINT TargetRootIndex{ File::ReadIntegerFromFile(InFile) };

            // <TargetObject>
            File::ReadStringFromFile(InFile, Token);

            UINT TargetObjectCount{ File::ReadIntegerFromFile(InFile) };

            if (TargetObjectCount > 0)
            {
                for (UINT i = 0; i < TargetObjectCount; ++i)
                {
                    File::ReadStringFromFile(InFile, Token);

                    shared_ptr<CGameObject> TargetObject{ m_GameObjects[OBJECT_TYPE_STRUCTURE][TargetRootIndex]->FindFrame(Token) };

                    EventTrigger->InsertEventObject(TargetObject);
                }
            }

            m_EventTriggers.push_back(EventTrigger);
        }
        else if (Token == TEXT("</EventTriggers>"))
        {
            break;
        }
    }

    tcout << FileName << TEXT(" 로드 완료...") << endl << endl;

    //// 권총을 드롭하는 트리거를 추가한다.
    //// 권총은 열쇠를 갖지 않은 임의의 교도관(Index: 2 ~ 14) 중 5명이 보유하고 있다.
    //vector<UINT> Indices{};

    //Indices.resize(m_GameObjects[OBJECT_TYPE_NPC].size() - 2);
    //iota(Indices.begin(), Indices.end(), 2);
    //shuffle(Indices.begin(), Indices.end(), default_random_engine{ random_device{}() });

    //for (UINT i = 0; i < 5; ++i)
    //{
    //    if (m_GameObjects[OBJECT_TYPE_NPC][Indices[i]])
    //    {
    //        shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(m_GameObjects[OBJECT_TYPE_NPC][Indices[i]]) };

    //        EventTrigger = make_shared<CGetPistolEventTrigger>();
    //        Guard->SetEventTrigger(EventTrigger);

    //        m_EventTriggers.push_back(EventTrigger);
    //    }
    //}

    //// 열쇠를 드롭하는 트리거를 추가한다.
    //// 열쇠는 외형이 다른 교도관(Index: 0, 1)이 보유하고 있다.
    //for (UINT i = 0; i < 2; ++i)
    //{
    //    if (m_GameObjects[OBJECT_TYPE_NPC][i])
    //    {
    //        shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(m_GameObjects[OBJECT_TYPE_NPC][i]) };

    //        EventTrigger = make_shared<CGetKeyEventTrigger>();
    //        Guard->SetEventTrigger(EventTrigger);

    //        m_EventTriggers.push_back(EventTrigger);
    //    }
    //}

    //// 모든 트리거 객체는 상호작용 UI 객체를 공유한다.
    //UINT TriggerCount{ static_cast<UINT>(m_EventTriggers.size()) };

    //for (const auto& EventTrigger : m_EventTriggers)
    //{
    //    if (EventTrigger)
    //    {
    //        // [BILBOARD_OBJECT_TYPE_UI][9]: InteractionUI
    //        EventTrigger->SetInteractionUI(m_BilboardObjects[BILBOARD_OBJECT_TYPE_UI][9]);
    //    }
    //}
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

    return true;
}

void CServer::RemovePlayer(UINT ID)
{
    closesocket(m_ClientSocketInfos[ID].m_Socket);
    SetEvent(m_ClientSyncEvents[ID]);

    m_ClientSocketInfos[ID].m_Socket = NULL;

    cout << "[클라이언트 종료] " << "IP : " << inet_ntoa(m_ClientSocketInfos[ID].m_SocketAddress.sin_addr) << ", 포트번호 : " << ntohs(m_ClientSocketInfos[ID].m_SocketAddress.sin_port) << endl;
}

void CServer::GameLoop()
{
    while (true)
    {
        ResetEvent(m_MainSyncEvents[1]);
        SetEvent(m_MainSyncEvents[0]);

        for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i)
        {
            if (m_ClientSocketInfos[i].m_Socket)
            {
                WaitForSingleObject(m_ClientSyncEvents[i], INFINITE);
            }
        }

        m_Timer->Tick(0.0f);

        UpdatePlayerInfo();
        Animate(m_Timer->GetElapsedTime());
        UpdateSendedPacketData();

        ResetEvent(m_MainSyncEvents[0]);
        SetEvent(m_MainSyncEvents[1]);
    }
}

void CServer::UpdatePlayerInfo()
{
    for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i)
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
    for (UINT i = OBJECT_TYPE_PLAYER; i <= OBJECT_TYPE_STRUCTURE; ++i)
    {
        for (const auto& GameObject : m_GameObjects[i])
        {
            if (GameObject)
            {
                GameObject->Animate(ElapsedTime);
                GameObject->UpdateTransform(Matrix4x4::Identity());
            }
        }
    }

    // 상호작용이 일어난 모든 트리거의 작업을 수행한다.
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
		// 광원 포지션과 방향벡터를 활용해 평면에 도달하는 중심점을 계산한다. 
		float LightAngle{ Vector3::Angle(m_Lights[0].m_Direction, XMFLOAT3(0.0f, -1.0f, 0.0f)) };  // 빗변과 변의 각도 계산
		float HypotenuseLength{ m_Lights[0].m_Position.y / cosf(XMConvertToRadians(LightAngle)) }; // 빗변의 길이 계산
		float Radian{ HypotenuseLength * tanf(XMConvertToRadians(10.0f)) };                        // 광원이 쏘아지는 원의 반지름

		// 평면에 도달하는 점 계산
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
										// 스팟조명과 충돌 할 경우 주변 범위에 있는 경찰들이 플레이어를 쫒기 시작한다.
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
    m_SendedPacketData.m_MsgType = m_MsgType;
    m_MsgType = MSG_TYPE_NONE;

    for (UINT i = 0; i < MAX_CLIENT_CAPACITY; ++i)
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
