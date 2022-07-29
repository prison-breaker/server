#include "stdafx.h"
#include "Server.h"

bool									CServer::m_IsGameClear{};
bool									CServer::m_InvincibleMode{};

MSG_TYPE								CServer::m_MsgType{};

vector<vector<shared_ptr<CGameObject>>> CServer::m_GameObjects{};
vector<shared_ptr<CEventTrigger>>       CServer::m_EventTriggers{};
shared_ptr<CNavMesh>                    CServer::m_NavMesh{};
vector<LIGHT>                           CServer::m_Lights{};

INIT_GAME_DATA					        CServer::m_InitGameData{};

SERVER_TO_CLIENT_DATA                   CServer::m_SendedPacketData{};

PLAYER_ATTACK_DATA					    CServer::m_PlayerAttackData{};
GUARD_ATTACK_DATA					    CServer::m_GuardAttackData{};
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

        cout << "[�� Ŭ���̾�Ʈ ����] " << "IP : " << inet_ntoa(ClientAddress.sin_addr) << ", ��Ʈ��ȣ : " << ntohs(ClientAddress.sin_port) << endl;
        cout << "    m_SceneType : " << Server->m_SceneType << endl;
        cout << "    m_RecentClientID : " << Server->m_RecentClientID << endl;

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

    while (true)
    {
        WaitForSingleObject(Server->m_MainSyncEvents[0], INFINITE);

        // �޼��� ��Ŷ �����͸� �����Ѵ�.
        ReturnValue = recv(ClientSocket, (char*)&Server->m_ReceivedMsgTypes[ClientID], sizeof(Server->m_ReceivedMsgTypes[ClientID]), MSG_WAITALL);

        if (ReturnValue == SOCKET_ERROR)
        {
            Server::ErrorDisplay("recv()");
            break;
        }
        else if (ReturnValue == 0)
        {
            break;
        }

        if (Server->m_ReceivedMsgTypes[ClientID] & MSG_TYPE_TITLE)
        {
            SetEvent(Server->m_ClientSyncEvents[ClientID]);
            WaitForSingleObject(Server->m_MainSyncEvents[1], INFINITE);

            ReturnValue = send(ClientSocket, (char*)&Server->m_SendedPacketData.m_MsgType, sizeof(Server->m_SendedPacketData.m_MsgType), 0);

            if (ReturnValue == SOCKET_ERROR)
            {
                Server::ErrorDisplay("send()");
                break;
            }
        }
        else if (Server->m_ReceivedMsgTypes[ClientID] & MSG_TYPE_INGAME)
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
                CAMERA_DATA CameraData{};

                ReturnValue = recv(ClientSocket, (char*)&CameraData, sizeof(CameraData), MSG_WAITALL);

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

                Player->SetCameraData(CameraData);
            }

            SetEvent(Server->m_ClientSyncEvents[ClientID]);
            WaitForSingleObject(Server->m_MainSyncEvents[1], INFINITE);

            ReturnValue = send(ClientSocket, (char*)&Server->m_SendedPacketData, sizeof(Server->m_SendedPacketData), 0);

            if (ReturnValue == SOCKET_ERROR)
            {
                Server::ErrorDisplay("send()");
                break;
            }

            if (Server->m_SendedPacketData.m_MsgType & MSG_TYPE_PLAYER_ATTACK)
            {
                ReturnValue = send(ClientSocket, (char*)&Server->m_PlayerAttackData, sizeof(Server->m_PlayerAttackData), 0);

                if (ReturnValue == SOCKET_ERROR)
                {
                    Server::ErrorDisplay("send()");
                    break;
                }
            }

            if (Server->m_SendedPacketData.m_MsgType & MSG_TYPE_GUARD_ATTACK)
            {
                ReturnValue = send(ClientSocket, (char*)&Server->m_GuardAttackData, sizeof(Server->m_GuardAttackData), 0);

                if (ReturnValue == SOCKET_ERROR)
                {
                    Server::ErrorDisplay("send()");
                    break;
                }
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
        else if (Server->m_ReceivedMsgTypes[ClientID] & MSG_TYPE_ENDING)
        {
            SetEvent(Server->m_ClientSyncEvents[ClientID]);
            WaitForSingleObject(Server->m_MainSyncEvents[1], INFINITE);

            ReturnValue = send(ClientSocket, (char*)&Server->m_SendedPacketData.m_MsgType, sizeof(Server->m_SendedPacketData.m_MsgType), 0);

            if (ReturnValue == SOCKET_ERROR)
            {
                Server::ErrorDisplay("send()");
                break;
            }

            XMFLOAT4X4 TransformMatrixes[MAX_PLAYER_CAPACITY]{ Server->m_SendedPacketData.m_PlayerWorldMatrixes[0], Server->m_SendedPacketData.m_PlayerWorldMatrixes[1] };

            ReturnValue = send(ClientSocket, (char*)TransformMatrixes, sizeof(TransformMatrixes), 0);

            if (ReturnValue == SOCKET_ERROR)
            {
                Server::ErrorDisplay("send()");
                break;
            }
        }
    }

    Server->RemovePlayer(ClientID);

    return 0;
}

void CServer::CreateEvents()
{
    m_MainSyncEvents[0] = CreateEvent(NULL, TRUE, TRUE, NULL);
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
    m_Lights[0].m_SpotLightHeightAngle = -1.0f;
    m_Lights[0].m_Speed = 0.5f;
}

void CServer::LoadSceneInfoFromFile(const tstring& FileName)
{
    // Ÿ�� ����ŭ �� ������ ũ�⸦ ���Ҵ��Ѵ�.
    m_GameObjects.resize(OBJECT_TYPE_STRUCTURE + 1);

    unordered_map<tstring, shared_ptr<CMesh>> MeshCaches{};

    LoadMeshCachesFromFile(TEXT("MeshesAndMaterials/Meshes.bin"), MeshCaches);

    tifstream InFile{ FileName, ios::binary };
    tstring Token{};

    shared_ptr<LOADED_MODEL_INFO> ModelInfo{};

    UINT Type{};
    UINT PlayerID{};
    UINT GuardID{};

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
            Type = File::ReadIntegerFromFile(InFile);
        }
        else if (Token == TEXT("<Instances>"))
        {
            const UINT InstanceCount{ File::ReadIntegerFromFile(InFile) };

            // <IsActive>
            vector<UINT> IsActive(InstanceCount);

            File::ReadStringFromFile(InFile, Token);
            InFile.read(reinterpret_cast<TCHAR*>(IsActive.data()), sizeof(UINT) * InstanceCount);

            // <TransformMatrix>
            vector<XMFLOAT4X4> TransformMatrixes(InstanceCount);

            File::ReadStringFromFile(InFile, Token);
            InFile.read(reinterpret_cast<TCHAR*>(TransformMatrixes.data()), sizeof(XMFLOAT4X4) * InstanceCount);

            switch (Type)
            {
            case OBJECT_TYPE_PLAYER:
            {
                for (UINT i = 0; i < InstanceCount; ++i)
                {
                    shared_ptr<CPlayer> Player{ make_shared<CPlayer>() };

                    Player->SetID(PlayerID++);
                    Player->SetActive(IsActive[i]);
                    Player->SetChild(ModelInfo->m_Model);
                    Player->SetTransformMatrix(TransformMatrixes[i]);
                    Player->UpdateTransform(Matrix4x4::Identity());
                    Player->SetAnimationController(ModelInfo);
                    Player->Initialize();

                    m_GameObjects[Type].push_back(Player);
                    m_InitGameData.m_PlayerInitTransformMatrixes.push_back(TransformMatrixes[i]);
                }
            }
            break;
            case OBJECT_TYPE_NPC:
            {
                // <TargetPosition>
                vector<XMFLOAT3> TargetPositions(InstanceCount);

                File::ReadStringFromFile(InFile, Token);
                InFile.read(reinterpret_cast<TCHAR*>(TargetPositions.data()), sizeof(XMFLOAT3) * InstanceCount);

                for (UINT i = 0; i < InstanceCount; ++i)
                {
                    shared_ptr<CGuard> Guard{ make_shared<CGuard>() };

                    Guard->SetID(GuardID++);
                    Guard->SetActive(IsActive[i]);
                    Guard->SetChild(ModelInfo->m_Model);
                    Guard->SetTransformMatrix(TransformMatrixes[i]);
                    Guard->UpdateTransform(Matrix4x4::Identity());
                    Guard->SetAnimationController(ModelInfo);
                    Guard->FindPatrolNavPath(m_NavMesh, TargetPositions[i]);
                    Guard->Initialize();

                    m_GameObjects[Type].push_back(Guard);
                    m_InitGameData.m_NPCInitTransformMatrixes.push_back(TransformMatrixes[i]);
                }
            }
            break;
            case OBJECT_TYPE_TERRAIN:
            case OBJECT_TYPE_STRUCTURE:
            {
                for (UINT i = 0; i < InstanceCount; ++i)
                {
                    // ���� �� ������ ��ü�� �����Ѵ�.
                    shared_ptr<CGameObject> Architecture{ make_shared<CGameObject>() };

                    Architecture->SetActive(IsActive[i]);
                    Architecture->SetChild(ModelInfo->m_Model);
                    Architecture->SetTransformMatrix(TransformMatrixes[i]);
                    Architecture->UpdateTransform(Matrix4x4::Identity());
                    Architecture->Initialize();

                    m_GameObjects[Type].push_back(Architecture);
                }
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

    SetEvent(m_ClientSyncEvents[ValidID]);

    return true;
}

void CServer::RemovePlayer(UINT ID)
{
    closesocket(m_ClientSocketInfos[ID].m_Socket);
    SetEvent(m_ClientSyncEvents[ID]);

    cout << "[�� Ŭ���̾�Ʈ ����] " << "IP : " << inet_ntoa(m_ClientSocketInfos[ID].m_SocketAddress.sin_addr) << ", ��Ʈ��ȣ : " << ntohs(m_ClientSocketInfos[ID].m_SocketAddress.sin_port) << endl;

    memset(&m_ClientSocketInfos[ID], NULL, sizeof(SOCKET_INFO));
    m_ReceivedMsgTypes[ID] = MSG_TYPE_NONE;
}

bool CServer::CheckConnection()
{
    for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
    {
        if (m_ReceivedMsgTypes[i] == MSG_TYPE_NONE)
        {
            return false;
        }
    }

    return true;
}

bool CServer::CheckAllPlayerReady()
{
    for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
    {
        if (m_ReceivedMsgTypes[i] != MSG_TYPE_TITLE)
        {
            return false;
        }
    }

    return true;
}

void CServer::CheckGameOver()
{
    if (!m_IsGameOver && !m_IsGameClear)
    {
        for (const auto& GameObject : m_GameObjects[OBJECT_TYPE_PLAYER])
        {
            if (GameObject)
            {
                shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(GameObject) };

                if (Player->GetHealth() <= 0)
                {
                    m_IsGameOver = true;
                }
            }
        }
    }
    else
    {
        if (m_IsGameOver)
        {
            m_ElapsedTimeFromGameOver += m_Timer->GetElapsedTime();

            if (m_ElapsedTimeFromGameOver >= 5.0f)
            {
                m_ElapsedTimeFromGameOver = 0.0f;
                m_SceneType = SCENE_TYPE_TITLE;

                ResetGameData();
            }
        }
        else if (m_IsGameClear)
        {
            m_SceneType = SCENE_TYPE_ENDING;

            for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
            {
                if (m_GameObjects[OBJECT_TYPE_PLAYER][i])
                {
                    shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(m_GameObjects[OBJECT_TYPE_PLAYER][i]) };

                    Player->SetLook(XMFLOAT3(0.0f, 0.0f, 1.0f));
                    Player->SetRight(XMFLOAT3(1.0f, 0.0f, 0.0f));
                    Player->SetUp(XMFLOAT3(0.0f, 1.0f, 0.0f));
                    Player->SetPosition(XMFLOAT3(-5.0f + 10.0f * i, 1.05f, -45.0f - 5.0f * i));
                    Player->GetStateMachine()->SetCurrentState(CPlayerRunningState::GetInstance());
                }
            }
        }
    }
}

void CServer::CheckEndingOver()
{
    m_ElapsedTimeFromEnding += m_Timer->GetElapsedTime();

    if (m_ElapsedTimeFromEnding >= 35.0f )
    {
        m_ElapsedTimeFromEnding = 0.0f;
        m_SceneType = SCENE_TYPE_CREDIT;
    }  
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

        m_Timer->Tick(60.0f);

        switch (m_SceneType)
        {
        case SCENE_TYPE_TITLE:
            if (CheckAllPlayerReady())
            {
                m_SceneType = SCENE_TYPE_INGAME;
            }
            break;
        case SCENE_TYPE_INGAME:
            ProcessInput();
            UpdatePlayerInfo();
            Animate(m_Timer->GetElapsedTime());
            CheckGameOver();
            break;
        case SCENE_TYPE_ENDING:
            UpdatePlayerInfo();
            Animate(m_Timer->GetElapsedTime());
            CheckEndingOver();
            break;
        }

        UpdateSendedPacketData();

        ResetEvent(m_MainSyncEvents[0]);
        SetEvent(m_MainSyncEvents[1]);
    }
}

void CServer::ResetGameData()
{
    for (const auto& EventTrigger : m_EventTriggers)
    {
        if (EventTrigger)
        {
            EventTrigger->Reset();
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
        m_HasPistolGuardIndices[i] = Indices[i];
    }

    for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
    {
        if (m_GameObjects[OBJECT_TYPE_PLAYER][i])
        {
            m_GameObjects[OBJECT_TYPE_PLAYER][i]->Reset(m_InitGameData.m_PlayerInitTransformMatrixes[i]);
        }
    }

    // 0 ~ 1: Has Key Guard
    // Random 5 : Has Pistol Guard
    for (UINT i = 0, j = 0; i < MAX_NPC_COUNT; ++i)
    {
        if (m_GameObjects[OBJECT_TYPE_NPC][i])
        {
            m_GameObjects[OBJECT_TYPE_NPC][i]->Reset(m_InitGameData.m_NPCInitTransformMatrixes[i]);

            if (i <= 1 || i == Indices[j - 2])
            {
                shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(m_GameObjects[OBJECT_TYPE_NPC][i]) };

                Guard->SetEventTrigger(m_EventTriggers[j++]);
            }
        }
    }

    m_InvincibleMode = false;

    m_Lights[0].m_SpotLightAngle = XMConvertToRadians(90.0f);
    m_Lights[0].m_Direction = Vector3::Normalize(XMFLOAT3(cosf(m_Lights[0].m_SpotLightAngle), -1.0f, sinf(m_Lights[0].m_SpotLightAngle)));

    memset(&m_SendedPacketData, 0, sizeof(m_SendedPacketData));
    memset(&m_TriggerData, 0, sizeof(m_TriggerData));

    m_IsGameOver = false;
    m_IsGameClear = false;
    m_ElapsedTimeFromGameOver = 0.0f;
    m_ElapsedTimeFromEnding = 0.0f;
    m_RecentClientID = 0;

    for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
    {
        m_ReceivedPacketData[i].m_InputMask = INPUT_MASK_NONE;
        m_ReceivedPacketData[i].m_WorldMatrix = m_GameObjects[OBJECT_TYPE_PLAYER][i]->GetWorldMatrix();
    }

    memset(m_ReceivedMsgTypes, 0, sizeof(m_ReceivedMsgTypes));

    tcout << "========== ���� ������ ���� �� ============" << endl;
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

    if (GetAsyncKeyState('C') & 0x0001)
    {
        m_IsGameClear = true;

        tcout << TEXT("������ Ŭ�����մϴ�.") << endl;
    }
}

void CServer::UpdatePlayerInfo()
{
    switch (m_SceneType)
    {
    case SCENE_TYPE_INGAME:
        for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
        {
            m_PlayerAttackData.m_TargetIndices[i] = UINT_MAX;
            m_TriggerData.m_TargetIndices[i] = UINT_MAX;
        }

        for (UINT i = 0; i < MAX_NPC_COUNT; ++i)
        {
            m_GuardAttackData.m_TargetIndices[i] = UINT_MAX;
        }

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
        break;
    case SCENE_TYPE_ENDING:
        for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
        {
            if (m_GameObjects[OBJECT_TYPE_PLAYER][i])
            {
                m_GameObjects[OBJECT_TYPE_PLAYER][i]->Move(XMFLOAT3(0.0f, 0.0f, 1.0f), 7.0f * m_Timer->GetElapsedTime());
            }
        }
        break;
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
        XMFLOAT3 ToLightedPosition{ Vector3::Subtract(LightedPosition, XMFLOAT3{0.0f, 0.0f, 0.0f}) };
        float ToLightedLength{ Vector3::Length(ToLightedPosition) };

        if (ToLightedLength < 90.0f)
        {
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
                                    shared_ptr<CGameObject> IntersectedObject{ GameObject->PickObjectByRayIntersection(m_Lights[0].m_Position, Direction, HitDistance, HypotenuseLength, false) };

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

                                // �÷��̾ ã�� ��� ����ž�� �÷��̾ �Ѿư���
                                XMFLOAT3 ToPlayerPosition{ Vector3::Subtract(Player->GetPosition(), XMFLOAT3{0.0f, 0.0f, 0.0f}) };
                                float ScalarProduct{ Vector3::CrossProduct(ToPlayerPosition, ToLightedPosition, true).y };

                                //�¿� ������
                                if (ScalarProduct > 0.5f)
                                {
                                    m_Lights[0].m_SpotLightAngle += m_Timer->GetElapsedTime() * m_Lights[0].m_Speed;
                                }
                                else if (ScalarProduct < -0.5f)
                                {
                                    m_Lights[0].m_SpotLightAngle -= m_Timer->GetElapsedTime() * m_Lights[0].m_Speed;
                                }

                                float Subtract{ ToLightedLength - Vector3::Length(ToPlayerPosition) };

                                //���� ������
                                if (Subtract > 1.0f)
                                {
                                    m_Lights[0].m_SpotLightHeightAngle -= m_Timer->GetElapsedTime() * m_Lights[0].m_Speed;
                                }
                                else if (Subtract < -1.0f)
                                {
                                    m_Lights[0].m_SpotLightHeightAngle += m_Timer->GetElapsedTime() * m_Lights[0].m_Speed;
                                }

                                UINT PlayerID = Player->GetID();

                                if (!m_IsInSpotLight[PlayerID])
                                {
                                    if (PlayerID == 0)
                                    {
                                        m_MsgType |= MSG_TYPE_PLAYER1_BGM_SWAP;
                                    }
                                    else
                                    {
                                        m_MsgType |= MSG_TYPE_PLAYER2_BGM_SWAP;
                                    }

                                    m_IsInSpotLight[PlayerID] = true;
                                }

                                m_Lights[0].m_Direction = Vector3::Normalize(XMFLOAT3(cosf(m_Lights[0].m_SpotLightAngle), m_Lights[0].m_SpotLightHeightAngle, sinf(m_Lights[0].m_SpotLightAngle)));
                                return;
                            }
                        }
                    }
                }
            }
        }
        
        //���� ���� ��ġ�� ���ư���
        if (m_Lights[0].m_SpotLightHeightAngle > -1.0f)
        {
            m_Lights[0].m_SpotLightHeightAngle -= m_Timer->GetElapsedTime() * m_Lights[0].m_Speed;

            if (m_Lights[0].m_SpotLightHeightAngle < -1.0f)
            {
                m_Lights[0].m_SpotLightHeightAngle = -1.0f;
            }
        }
        else if (m_Lights[0].m_SpotLightHeightAngle < -1.0f)
        {
            m_Lights[0].m_SpotLightHeightAngle += m_Timer->GetElapsedTime() * m_Lights[0].m_Speed;

            if (m_Lights[0].m_SpotLightHeightAngle > -1.0f)
            {
                m_Lights[0].m_SpotLightHeightAngle = -1.0f;
            }
        }

        for (UINT i = 0; i < MAX_PLAYER_CAPACITY; i++)
        {
            if (m_IsInSpotLight[i])
            {
                if (i == 0)
                {
                    m_MsgType |= MSG_TYPE_PLAYER1_BGM_SWAP;
                }
                else
                {
                    m_MsgType |= MSG_TYPE_PLAYER2_BGM_SWAP;
                }
                m_IsInSpotLight[i] = false;
            }
        }

        m_Lights[0].m_SpotLightAngle += m_Timer->GetElapsedTime() * m_Lights[0].m_Speed;
		m_Lights[0].m_Direction = Vector3::Normalize(XMFLOAT3(cosf(m_Lights[0].m_SpotLightAngle), m_Lights[0].m_SpotLightHeightAngle, sinf(m_Lights[0].m_SpotLightAngle)));
	}
}

void CServer::UpdateSendedPacketData()
{
    switch (m_SceneType)
    {
    case SCENE_TYPE_TITLE:
        m_MsgType = MSG_TYPE_TITLE;
        break;
    case SCENE_TYPE_INGAME:
        if (!CheckConnection())
        {
            m_SceneType = SCENE_TYPE_TITLE;
            m_MsgType = MSG_TYPE_DISCONNECTION;

            ResetGameData();

            break;
        }

        for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
        {
            m_SendedPacketData.m_PlayerWorldMatrixes[i] = m_GameObjects[OBJECT_TYPE_PLAYER][i]->GetWorldMatrix();
            m_SendedPacketData.m_PlayerAnimationClipTypes[i] = m_GameObjects[OBJECT_TYPE_PLAYER][i]->GetAnimationController()->GetAnimationClipType();
        }

        for (UINT i = 0; i < MAX_NPC_COUNT; ++i)
        {
            m_SendedPacketData.m_NPCWorldMatrixes[i] = m_GameObjects[OBJECT_TYPE_NPC][i]->GetWorldMatrix();
            m_SendedPacketData.m_NPCAnimationClipTypes[i] = m_GameObjects[OBJECT_TYPE_NPC][i]->GetAnimationController()->GetAnimationClipType();
        }

        m_SendedPacketData.m_TowerLightDirection = m_Lights[0].m_Direction;
        m_MsgType |= MSG_TYPE_INGAME;

        if (m_IsGameOver)
        {
            m_MsgType |= MSG_TYPE_GAME_OVER;
        }
        else if (m_IsGameClear)
        {
            m_MsgType |= MSG_TYPE_GAME_CLEAR;
        }
        break;
    case SCENE_TYPE_ENDING:
        if (!CheckConnection())
        {
            m_SceneType = SCENE_TYPE_TITLE;
            m_MsgType = MSG_TYPE_DISCONNECTION;

            ResetGameData();

            break;
        }

        for (UINT i = 0; i < MAX_PLAYER_CAPACITY; ++i)
        {
            m_SendedPacketData.m_PlayerWorldMatrixes[i] = m_GameObjects[OBJECT_TYPE_PLAYER][i]->GetWorldMatrix();
            m_SendedPacketData.m_PlayerAnimationClipTypes[i] = m_GameObjects[OBJECT_TYPE_PLAYER][i]->GetAnimationController()->GetAnimationClipType();
        }

        m_MsgType |= MSG_TYPE_ENDING;
        break;
    case SCENE_TYPE_CREDIT:
        m_MsgType = MSG_TYPE_CREDIT;
        m_SceneType = SCENE_TYPE_TITLE;
        break;
    }

    m_SendedPacketData.m_MsgType = m_MsgType;
    m_MsgType = MSG_TYPE_NONE;
}
