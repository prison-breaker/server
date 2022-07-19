#include "stdafx.h"
#include "Guard.h"
#include "Server.h"

void CGuard::Initialize()
{
	CGameObject::Initialize();

	// 상태머신 객체를 생성한다.
	m_StateMachine = make_shared<CStateMachine<CGuard>>(static_pointer_cast<CGuard>(shared_from_this()));
	m_StateMachine->SetCurrentState(CGuardPatrolState::GetInstance());
}

void CGuard::Reset(const XMFLOAT4X4& TransformMatrix)
{
	m_IsActive = true;
	m_Health = 100;
	m_NavPath.clear();
	m_PatrolIndex = 0;
	m_EventTrigger = nullptr;

	SetTransformMatrix(TransformMatrix);
	UpdateTransform(Matrix4x4::Identity());

	m_StateMachine->SetCurrentState(CGuardPatrolState::GetInstance());
}

void CGuard::Animate(float ElapsedTime)
{
	if (IsActive())
	{
		if (m_StateMachine)
		{
			m_StateMachine->Update(ElapsedTime);
		}
	}
}

void CGuard::SetID(UINT ID)
{
	m_ID = ID;
}

UINT CGuard::GetID() const
{
	return m_ID;
}

void CGuard::SetHealth(UINT Health)
{
	// UINT UnderFlow
	if (Health > 100)
	{
		m_Health = 0;
	}
	else
	{
		m_Health = Health;
	}
}

UINT CGuard::GetHealth() const
{
	return m_Health;
}

void CGuard::SetSpeed(float Speed)
{
	m_Speed = Speed;
}

float CGuard::GetSpeed() const
{
	return m_Speed;
}

void CGuard::SetMovingDirection(const XMFLOAT3& MovingDirection)
{
	m_MovingDirection = MovingDirection;
}

const XMFLOAT3& CGuard::GetMovingDirection() const
{
	return m_MovingDirection;
}

shared_ptr<CStateMachine<CGuard>> CGuard::GetStateMachine() const
{
	return m_StateMachine;
}

void CGuard::SetRecentTransition(bool RecentTransition)
{
	m_RecentTransition = RecentTransition;
}

bool CGuard::GetRecentTransition() const
{
	return m_RecentTransition;
}

void CGuard::SetElapsedTime(float ElapsedTime)
{
	m_ElapsedTime = ElapsedTime;

	if (m_ElapsedTime >= 3.0f)
	{
		m_RecentTransition = true;
	}
}

float CGuard::GetElapsedTime() const
{
	return m_ElapsedTime;
}

float CGuard::GetToIdleEntryTime() const
{
	return m_ToIdleEntryTime;
}

float CGuard::GetUpdateTargetTime() const
{
	return m_UpdateTargetTime;
}

void CGuard::SetTarget(const shared_ptr<CGameObject>& Target)
{
	m_Target = Target;
}

shared_ptr<CGameObject> CGuard::GetTarget() const
{
	return m_Target;
}

void CGuard::SetEventTrigger(const shared_ptr<CEventTrigger>& EventTrigger)
{
	m_EventTrigger = EventTrigger;
}

shared_ptr<CEventTrigger> CGuard::GetEventTrigger()
{
	return m_EventTrigger;
}

vector<XMFLOAT3>& CGuard::GetNavPath()
{
	return m_NavPath;
}

vector<XMFLOAT3>& CGuard::GetPatrolNavPath()
{
	return m_PatrolNavPath;
}

UINT CGuard::GetPatrolIndex() const
{
	return m_PatrolIndex;
}

shared_ptr<CGameObject> CGuard::IsFoundPlayer(const vector<vector<shared_ptr<CGameObject>>>& GameObjects) const
{
	shared_ptr<CGameObject> NearestPlayer{};
	float NearestDistance{ FLT_MAX };
	float Distance{};

	// 더 가까운 플레이어를 찾는다.
	for (const auto& GameObject : GameObjects[OBJECT_TYPE_PLAYER])
	{
		if (GameObject)
		{
			shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(GameObject) };

			if (Player->GetHealth() > 0)
			{
				XMFLOAT3 ToPlayer{ Vector3::Subtract(Player->GetPosition(), GetPosition()) };

				Distance = Vector3::Length(ToPlayer);

				if (Distance < 30.0f)
				{
					float BetweenDegree{ Vector3::Angle(Vector3::Normalize(GetLook()), Vector3::Normalize(ToPlayer)) };

					if (BetweenDegree < 100.0f)
					{
						float NearestHitDistance{ FLT_MAX };
						float HitDistance{};
						bool IsHit{};

						for (const auto& GameObject : GameObjects[OBJECT_TYPE_STRUCTURE])
						{
							if (GameObject)
							{
								XMFLOAT3 RayOrigin{ GetPosition() };
								RayOrigin.y = 5.0f;

								shared_ptr<CGameObject> IntersectedObject{ GameObject->PickObjectByRayIntersection(RayOrigin, Vector3::Normalize(ToPlayer), HitDistance, 30.0f)};

								if (IntersectedObject && HitDistance < 30.0f)
								{
									IsHit = true;
									break;
								}
							}
						}

						if (!IsHit)
						{
							if (Distance < NearestDistance)
							{
								NearestPlayer = Player;
								NearestDistance = Distance;
							}
						}
					}
				}
			}
		}
	}

	return NearestPlayer;
}

void CGuard::FindNavPath(const shared_ptr<CNavMesh>& NavMesh, const XMFLOAT3& TargetPosition, const vector<vector<shared_ptr<CGameObject>>>& GameObjects)
{
	priority_queue<shared_ptr<CNavNode>, vector<shared_ptr<CNavNode>>, compare> NavNodeQueue{};

	UINT StartNavNodeIndex{ NavMesh->GetNodeIndex(GetPosition()) };
	UINT TargetNavNodeIndex{ NavMesh->GetNodeIndex(TargetPosition) };

	shared_ptr<CNavNode> StartNavNode{ NavMesh->GetNavNodes()[StartNavNodeIndex] };
	shared_ptr<CNavNode> TargetNavNode{ NavMesh->GetNavNodes()[TargetNavNodeIndex] };

	for (const auto& NavNode : NavMesh->GetNavNodes())
	{
		NavNode->Reset();
	}

	StartNavNode->CalculateF(StartNavNode, TargetNavNode);
	NavNodeQueue.push(StartNavNode);

	shared_ptr<CNavNode> CurrentNavNode{};
	UINT CurrentIndex{};

	while (true)
	{
		if (NavNodeQueue.empty())
		{
			//tcout << TEXT("목적지를 찾지 못했습니다.") << endl;
			break;
		}

		CurrentNavNode = NavNodeQueue.top();
		NavNodeQueue.pop();

		if (CurrentNavNode->IsVisited())
		{
			continue;
		}

		CurrentNavNode->SetVisit(true);

		if (Vector3::IsEqual(CurrentNavNode->GetTriangle().m_Centroid, TargetNavNode->GetTriangle().m_Centroid))
		{
			//tcout << TEXT("목적지를 찾았습니다.") << endl;
			break;
		}

		for (const auto& NeighborNavNode : CurrentNavNode->GetNeighborNavNodes())
		{
			if (!NeighborNavNode->IsVisited())
			{
				NeighborNavNode->SetParent(CurrentNavNode);
				NeighborNavNode->CalculateF(CurrentNavNode, TargetNavNode);
				NavNodeQueue.push(NeighborNavNode);
			}
		}
	}

	m_NavPath.clear();
	m_NavPath.push_back(TargetPosition);

	while (true)
	{
		if (!CurrentNavNode->GetParent())
		{
			m_NavPath.push_back(GetPosition());
			break;
		}

		UINT NeighborSideIndex{ CurrentNavNode->CalculateNeighborSideIndex(CurrentNavNode->GetParent()) };

		m_NavPath.push_back(CurrentNavNode->GetTriangle().m_CenterSides[NeighborSideIndex]);
		CurrentNavNode = CurrentNavNode->GetParent();
	}

	// 경로를 만들었다면, 노드를 순회하며 광선을 쏴 더 최적화된 경로를 구한다.
	FindRayCastingNavPath(GameObjects);
}

void CGuard::FindRayCastingNavPath(const vector<vector<shared_ptr<CGameObject>>>& GameObjects)
{
	vector<XMFLOAT3> RayCastingNavPath{};

	UINT NavPathSize{ static_cast<UINT>(m_NavPath.size()) };

	RayCastingNavPath.push_back(m_NavPath.front());

	for (UINT SearchIndex = 0; SearchIndex < NavPathSize - 1; ++SearchIndex)
	{
		XMFLOAT3 SearchPosition{ m_NavPath[SearchIndex] };

		for (UINT CheckIndex = NavPathSize - 1; CheckIndex > SearchIndex; --CheckIndex)
		{
			XMFLOAT3 CheckPosition{ m_NavPath[CheckIndex] };

			// 바운딩 박스와 충돌이 일어나기 위한 최소 높이이다.
			SearchPosition.y = CheckPosition.y = 5.0f;

			// Normalize 안하니까 터지지? 반대 방향은 왜 안되는지?
			XMFLOAT3 ToNextPosition{ Vector3::Subtract(SearchPosition, CheckPosition) }; 
			float Distance{ Vector3::Length(ToNextPosition) };
			float HitDistance{};
			bool IsHit{};

			for (const auto& GameObject : GameObjects[OBJECT_TYPE_STRUCTURE])
			{
				if (GameObject)
				{
					shared_ptr<CGameObject> IntersectedObject{ GameObject->PickObjectByRayIntersection(CheckPosition, Vector3::Normalize(ToNextPosition), HitDistance, Distance) };

					if (IntersectedObject && (HitDistance < Distance))
					{
						IsHit = true;
						break;
					}
				}
			}

			if (!IsHit)
			{
				RayCastingNavPath.push_back(m_NavPath[CheckIndex]);
				SearchIndex = CheckIndex - 1;
				break;
			}
		}
	}

	if (!RayCastingNavPath.empty())
	{
		m_NavPath.clear();
		m_NavPath = RayCastingNavPath;
	}
	else
	{
		tcout << "터지는 이유" << endl;
	}
}

void CGuard::FindPatrolNavPath(const shared_ptr<CNavMesh>& NavMesh, const XMFLOAT3& TargetPosition)
{
	priority_queue<shared_ptr<CNavNode>, vector<shared_ptr<CNavNode>>, compare> NavNodeQueue{};

	UINT StartNavNodeIndex{ NavMesh->GetNodeIndex(GetPosition()) };
	UINT TargetNavNodeIndex{ NavMesh->GetNodeIndex(TargetPosition) };

	shared_ptr<CNavNode> StartNavNode{ NavMesh->GetNavNodes()[StartNavNodeIndex] };
	shared_ptr<CNavNode> TargetNavNode{ NavMesh->GetNavNodes()[TargetNavNodeIndex] };

	for (const auto& NavNode : NavMesh->GetNavNodes())
	{
		NavNode->Reset();
	}

	StartNavNode->CalculateF(StartNavNode, TargetNavNode);
	NavNodeQueue.push(StartNavNode);

	shared_ptr<CNavNode> CurrentNavNode{};
	UINT CurrentIndex{};

	while (true)
	{
		if (NavNodeQueue.empty())
		{
			//tcout << TEXT("목적지를 찾지 못했습니다.") << endl;
			break;
		}

		CurrentNavNode = NavNodeQueue.top();
		NavNodeQueue.pop();

		if (CurrentNavNode->IsVisited())
		{
			continue;
		}

		CurrentNavNode->SetVisit(true);

		if (Vector3::IsEqual(CurrentNavNode->GetTriangle().m_Centroid, TargetNavNode->GetTriangle().m_Centroid))
		{
			//tcout << TEXT("목적지를 찾았습니다.") << endl;
			break;
		}

		for (const auto& NeighborNavNode : CurrentNavNode->GetNeighborNavNodes())
		{
			if (!NeighborNavNode->IsVisited())
			{
				NeighborNavNode->SetParent(CurrentNavNode);
				NeighborNavNode->CalculateF(CurrentNavNode, TargetNavNode);
				NavNodeQueue.push(NeighborNavNode);
			}
		}
	}

	m_PatrolNavPath.clear();
	m_PatrolNavPath.push_back(TargetPosition);

	while (true)
	{
		if (!CurrentNavNode->GetParent())
		{
			m_PatrolNavPath.push_back(GetPosition());
			break;
		}

		UINT NeighborSideIndex{ CurrentNavNode->CalculateNeighborSideIndex(CurrentNavNode->GetParent()) };

		m_PatrolNavPath.push_back(CurrentNavNode->GetTriangle().m_CenterSides[NeighborSideIndex]);
		CurrentNavNode = CurrentNavNode->GetParent();
	}

	reverse(m_PatrolNavPath.begin(), m_PatrolNavPath.end());
}

shared_ptr<CGameObject> CGuard::IsCollidedByGuard(const XMFLOAT3& NewPosition)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

	for (const auto& GameObject : GameObjects[OBJECT_TYPE_NPC])
	{
		if (GameObject)
		{
			shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(GameObject) };

			if (Guard->GetHealth() > 0)
			{
				if (shared_from_this() != GameObject)
				{
					if (Math::Distance(Guard->GetPosition(), NewPosition) <= 2.0f)
					{
						return Guard;
					}
				}
			}
		}
	}

	return nullptr;
}

void CGuard::MoveToNavPath(float ElapsedTime)
{
	if (!m_NavPath.empty())
	{
		if (Math::Distance(m_NavPath.back(), GetPosition()) < 1.0f)
		{
			m_NavPath.pop_back();

			if (!m_NavPath.empty())
			{
				m_MovingDirection = Vector3::Normalize(Vector3::Subtract(m_NavPath.back(), GetPosition()));
				
				UpdateLocalCoord(m_MovingDirection);
			}
		}
		else
		{
			XMFLOAT3 NewPosition{ Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_Speed * ElapsedTime, m_MovingDirection, false)) };
			shared_ptr<CGameObject> CollidedGuard{ IsCollidedByGuard(NewPosition) };

			if (CollidedGuard)
			{
				XMFLOAT3 ToGuard{ Vector3::Subtract(CollidedGuard->GetPosition(), GetPosition()) };
				float ScalarProduct{ Vector3::CrossProduct(GetLook(), ToGuard, false).y };

				if (ScalarProduct > 0.0f)
				{
					NewPosition = Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_Speed * ElapsedTime, Vector3::Inverse(GetRight()), false));
				}
				else
				{
					NewPosition = Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_Speed * ElapsedTime, GetRight(), false));
				}

				shared_ptr<CNavMesh> NavMesh{ CServer::m_NavMesh };

				if (!IsInNavMesh(NavMesh, NewPosition))
				{
					NewPosition = GetPosition();
				}
				else
				{
					m_MovingDirection = Vector3::Normalize(Vector3::Subtract(m_NavPath.back(), NewPosition));

					UpdateLocalCoord(m_MovingDirection);
				}
			}

			SetPosition(NewPosition);
		}
	}
}

void CGuard::Patrol(float ElapsedTime)
{
	if (Math::Distance(m_PatrolNavPath[m_PatrolIndex], GetPosition()) < 1.0f)
	{
		m_PatrolIndex += 1;

		if (m_PatrolIndex == m_PatrolNavPath.size())
		{
			m_PatrolIndex = 0;

			reverse(m_PatrolNavPath.begin(), m_PatrolNavPath.end());
		}

		m_MovingDirection = Vector3::Normalize(Vector3::Subtract(m_PatrolNavPath[m_PatrolIndex], GetPosition()));

		UpdateLocalCoord(m_MovingDirection);
	}
	else
	{
		CGameObject::Move(m_MovingDirection, m_Speed * ElapsedTime);
	}
}

void CGuard::GenerateTrigger()
{
	if (m_EventTrigger)
	{
		m_EventTrigger->SetActive(true);
		m_EventTrigger->CalculateTriggerAreaByGuard(GetPosition());
	}
}
