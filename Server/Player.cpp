#include "stdafx.h"
#include "Player.h"
#include "EventTrigger.h"
#include "Server.h"

void CPlayer::Initialize()
{
	CGameObject::Initialize();

	// ���¸ӽ� ��ü�� �����Ѵ�.
	m_StateMachine = make_shared<CStateMachine<CPlayer>>(static_pointer_cast<CPlayer>(shared_from_this()));
	m_StateMachine->SetCurrentState(CPlayerIdleState::GetInstance());

	FindFrame(TEXT("gun_pr_1"))->SetActive(false);
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

shared_ptr<CStateMachine<CPlayer>> CPlayer::GetStateMachine() const
{
	return m_StateMachine;
}

void CPlayer::AcquirePistol()
{
	m_PistolFrame = FindFrame(TEXT("gun_pr_1"));
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

	// ������ NewPosition�� NavMesh ���� ���ٸ�, ���� ��ġ�� �����Ѵ�.
	if (!IsInNavMesh(NavMesh, NewPosition))
	{
		NewPosition = GetPosition();
		//tcout << TEXT("Not in NavMesh!") << endl;
	}
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

	for (auto iter = EventTriggers.begin(); iter != EventTriggers.end(); ++iter)
	{
		shared_ptr<CEventTrigger> EventTrigger = *iter;

		if (EventTrigger)
		{
			if (EventTrigger->IsInTriggerArea(GetPosition(), GetLook()))
			{
				if (IsInteracted)
				{
					EventTrigger->InteractEventTrigger();
				}

				if (EventTrigger->CanPassTriggerArea(GetPosition(), NewPosition))
				{
					return false;
				}
				else
				{
					return true;
				}
			}
		}
	}

	return false;
}

void CPlayer::ProcessInput(float ElapsedTime, UINT InputMask)
{
	if (m_StateMachine)
	{
		m_StateMachine->ProcessInput(ElapsedTime, InputMask);
	}

	shared_ptr<CNavMesh> NavMesh{ CServer::m_NavMesh };
	XMFLOAT3 NewPosition{ Vector3::Add(GetPosition(), Vector3::ScalarProduct(m_Speed * ElapsedTime, m_MovingDirection, false)) };

	if (!IsInNavMesh(NavMesh, NewPosition))
	{
		// SlidingVector�� �̿��Ͽ� NewPosition�� ���� �����Ѵ�.
		ApplySlidingVectorToPosition(NavMesh, NewPosition);
	}

	// NewPosition���� �̵� ��, ���������� �浹, Ʈ���� �� ��ȣ�ۿ��� ó���ϰų�, ������ ���� ������ ���� �ʴ´ٸ� �����̵��� �����.
	if (!IsCollidedByGuard(NewPosition) && !IsCollidedByEventTrigger(NewPosition, (InputMask & INPUT_MASK_F) ? true : false))
	{
		SetPosition(NewPosition);
	}
}