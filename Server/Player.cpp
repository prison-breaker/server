#include "stdafx.h"
#include "Player.h"
#include "EventTrigger.h"
#include "Server.h"

void CPlayer::Initialize()
{
	CGameObject::Initialize();

	// 상태머신 객체를 생성한다.
	m_StateMachine = make_shared<CStateMachine<CPlayer>>(static_pointer_cast<CPlayer>(shared_from_this()));
	m_StateMachine->SetCurrentState(CPlayerIdleState::GetInstance());

	FindFrame(TEXT("gun_pr_1"))->SetActive(false);
}

void CPlayer::Reset(const XMFLOAT4X4& TransformMatrix)
{
	SwapWeapon(WEAPON_TYPE_PUNCH);
	ManagePistol(false);

	m_Health = 100;
	m_StateMachine->SetCurrentState(CPlayerIdleState::GetInstance());

	SetTransformMatrix(TransformMatrix);
	UpdateTransform(Matrix4x4::Identity());
}

void CPlayer::Animate(float ElapsedTime)
{
	if (IsActive())
	{
		if (m_StateMachine)
		{
			m_StateMachine->Update(ElapsedTime);
		}
	}
}

void CPlayer::SetID(UINT ID)
{
	m_ID = ID;
}

UINT CPlayer::GetID() const
{
	return m_ID;
}

void CPlayer::SetHealth(UINT Health)
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

UINT CPlayer::GetHealth() const
{
	return m_Health;
}

void CPlayer::SetSpeed(float Speed)
{
	m_Speed = Speed;
}

float CPlayer::GetSpeed() const
{
	return m_Speed;
}

void CPlayer::SetMovingDirection(const XMFLOAT3& MovingDirection)
{
	m_MovingDirection = MovingDirection;
}

const XMFLOAT3& CPlayer::GetMovingDirection() const
{
	return m_MovingDirection;
}

void CPlayer::SetCameraPosition(const XMFLOAT3& CameraPosition)
{
	m_CameraPosition = CameraPosition;
}

const XMFLOAT3& CPlayer::GetCameraPosition() const
{
	return m_CameraPosition;
}

shared_ptr<CStateMachine<CPlayer>> CPlayer::GetStateMachine() const
{
	return m_StateMachine;
}

void CPlayer::ManagePistol(bool AcquirePistol)
{
	if (AcquirePistol)
	{
		m_PistolFrame = FindFrame(TEXT("gun_pr_1"));
	}
	else
	{
		m_PistolFrame = nullptr;
	}
}

bool CPlayer::HasPistol() const
{
	return (m_PistolFrame) ? true : false;
}

bool CPlayer::IsEquippedPistol() const
{
	if (m_PistolFrame)
	{
		return m_PistolFrame->IsActive();
	}

	return false;
}

bool CPlayer::SwapWeapon(WEAPON_TYPE WeaponType)
{
	bool IsSwapped{};

	if (m_PistolFrame)
	{
		switch (WeaponType)
		{
		case WEAPON_TYPE_PUNCH:
			m_PistolFrame->SetActive(false);
			IsSwapped = true;
			break;
		case WEAPON_TYPE_PISTOL:
			m_PistolFrame->SetActive(true);
			IsSwapped = true;
			break;
		}
	}

	return IsSwapped;
}

void CPlayer::ApplySlidingVectorToPosition(const shared_ptr<CNavMesh>& NavMesh, XMFLOAT3& NewPosition)
{
	XMFLOAT3 Shift{ Vector3::Subtract(NewPosition, GetPosition()) };

	shared_ptr<CNavNode> NavNode{ NavMesh->GetNavNodes()[NavMesh->GetNodeIndex(GetPosition())] };

	XMFLOAT3 SlidingVector{};
	XMFLOAT3 Vertices[3]{ NavNode->GetTriangle().m_Vertices[0], NavNode->GetTriangle().m_Vertices[1], NavNode->GetTriangle().m_Vertices[2] };
	XMFLOAT3 Edge{};
	XMFLOAT3 ContactNormal{};

	if (Math::LineIntersection(Vertices[0], Vertices[1], NewPosition, GetPosition()))
	{
		Edge = Vector3::Subtract(Vertices[0], Vertices[1]);
		ContactNormal = Vector3::Normalize(Vector3::TransformNormal(Edge, Matrix4x4::RotationYawPitchRoll(0.0f, 90.0f, 0.0f)));

		SlidingVector = Vector3::Subtract(Shift, Vector3::ScalarProduct(Vector3::DotProduct(Shift, ContactNormal), ContactNormal, false));
		NewPosition = Vector3::Add(GetPosition(), SlidingVector);
	}
	else if (Math::LineIntersection(Vertices[1], Vertices[2], NewPosition, GetPosition()))
	{
		Edge = Vector3::Subtract(Vertices[1], Vertices[2]);
		ContactNormal = Vector3::Normalize(Vector3::TransformNormal(Edge, Matrix4x4::RotationYawPitchRoll(0.0f, 90.0f, 0.0f)));

		SlidingVector = Vector3::Subtract(Shift, Vector3::ScalarProduct(Vector3::DotProduct(Shift, ContactNormal), ContactNormal, false));
		NewPosition = Vector3::Add(GetPosition(), SlidingVector);
	}
	else if (Math::LineIntersection(Vertices[2], Vertices[0], NewPosition, GetPosition()))
	{
		Edge = Vector3::Subtract(Vertices[2], Vertices[0]);
		ContactNormal = Vector3::Normalize(Vector3::TransformNormal(Edge, Matrix4x4::RotationYawPitchRoll(0.0f, 90.0f, 0.0f)));

		SlidingVector = Vector3::Subtract(Shift, Vector3::ScalarProduct(Vector3::DotProduct(Shift, ContactNormal), ContactNormal, false));
		NewPosition = Vector3::Add(GetPosition(), SlidingVector);
	}

	// 보정된 NewPosition도 NavMesh 위에 없다면, 기존 위치로 설정한다.
	if (!IsInNavMesh(NavMesh, NewPosition))
	{
		NewPosition = GetPosition();
		//tcout << TEXT("Not in NavMesh!") << endl;
	}
}

bool CPlayer::IsCollidedByPlayer(const XMFLOAT3& NewPosition)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

	for (const auto& GameObject : GameObjects[OBJECT_TYPE_PLAYER])
	{
		if (GameObject)
		{
			if (shared_from_this() != GameObject)
			{
				shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(GameObject) };

				if (Player->GetHealth() > 0)
				{
					if (Math::Distance(Player->GetPosition(), NewPosition) <= 2.0f)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool CPlayer::IsCollidedByGuard(const XMFLOAT3& NewPosition)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

	for (const auto& GameObject : GameObjects[OBJECT_TYPE_NPC])
	{
		if (GameObject)
		{
			shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(GameObject) };

			if (Guard->GetHealth() > 0)
			{
				if (Math::Distance(Guard->GetPosition(), NewPosition) <= 2.0f)
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool CPlayer::IsCollidedByEventTrigger(const XMFLOAT3& NewPosition, bool IsInteracted)
{
	vector<shared_ptr<CEventTrigger>>& EventTriggers{ CServer::m_EventTriggers };
	UINT TriggerCount{ static_cast<UINT>(EventTriggers.size()) };

	for (UINT i = 0; i < TriggerCount; ++i)
	{
		if (EventTriggers[i])
		{
			if (EventTriggers[i]->IsInTriggerArea(GetPosition(), GetLook()))
			{
				if (IsInteracted)
				{
					EventTriggers[i]->InteractEventTrigger(GetID());

					CServer::m_MsgType |= MSG_TYPE_TRIGGER;

					UINT Index{ CServer::m_TriggerData.m_Size++ };

					CServer::m_TriggerData.m_TriggerIndexStack[Index] = i;

					// 만약 트리거가 열쇠나 권총 획득 트리거라면, 습득한 플레이어의 인덱스 값을 기록한다.
					if (i <= 6)
					{
						CServer::m_TriggerData.m_CallerIndexStack[Index] = GetID();
					}
				}

				if (EventTriggers[i]->CanPassTriggerArea(GetPosition(), NewPosition))
				{
					return false;
				}

				return true;
			}
		}
	}

	return false;
}

void CPlayer::ProcessInput(float ElapsedTime, UINT InputMask)
{
	if (InputMask & INPUT_MASK_NUM1)
	{
		if (IsEquippedPistol())
		{
			if (SwapWeapon(WEAPON_TYPE_PUNCH))
			{
				switch (GetID())
				{
				case 0:
					CServer::m_MsgType |= MSG_TYPE_PLAYER1_WEAPON_SWAP;
					break;
				case 1:
					CServer::m_MsgType |= MSG_TYPE_PLAYER2_WEAPON_SWAP;
					break;
				}
			}
		}
	}

	if (InputMask & INPUT_MASK_NUM2)
	{
		if (!IsEquippedPistol())
		{
			if (SwapWeapon(WEAPON_TYPE_PISTOL))
			{
				switch (GetID())
				{
				case 0:
					CServer::m_MsgType |= MSG_TYPE_PLAYER1_WEAPON_SWAP;
					break;
				case 1:
					CServer::m_MsgType |= MSG_TYPE_PLAYER2_WEAPON_SWAP;
					break;
				}
			}
		}
	}

	if (m_StateMachine)
	{
		m_StateMachine->ProcessInput(ElapsedTime, InputMask);
	}

	shared_ptr<CNavMesh> NavMesh{ CServer::m_NavMesh };
	XMFLOAT3 NewPosition{ Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_Speed * ElapsedTime, m_MovingDirection, false)) };

	if (!IsInNavMesh(NavMesh, NewPosition))
	{
		// SlidingVector를 이용하여 NewPosition의 값을 보정한다.
		ApplySlidingVectorToPosition(NavMesh, NewPosition);
	}

	// NewPosition으로 이동 시, 타 플레이어, 교도관과의 충돌, 트리거 내 상호작용을 처리하거나, 움직임 제어 영향을 받지 않는다면 움직이도록 만든다.
	if (!IsCollidedByPlayer(NewPosition) && !IsCollidedByGuard(NewPosition) && !IsCollidedByEventTrigger(NewPosition, (InputMask & INPUT_MASK_F) ? true : false))
	{
		SetPosition(NewPosition);
	}
}
