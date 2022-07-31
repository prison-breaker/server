#include "stdafx.h"
#include "Server.h"
#include "Player.h"
#include "Guard.h"
#include "StateMachine.h"
#include "State_Guard.h"

COpenDoorEventTrigger::COpenDoorEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 70.0f;
}

void COpenDoorEventTrigger::Reset()
{
	CEventTrigger::Reset();

	const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

	m_EventObjects[0]->Rotate(WorldUp, -m_DoorAngle);
	m_EventObjects[1]->Rotate(WorldUp,  m_DoorAngle);
	m_DoorAngle = 0.0f;
}

bool COpenDoorEventTrigger::CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition)
{
	// 문 너머로 넘어가는 것을 계산한다.
	if (Math::LineIntersection(m_TriggerArea[0], m_TriggerArea[3], Position, NewPosition))
	{
		return false;
	}

	// 위 경우가 아니라면, 이동이 가능하다.
	return true;
}

void COpenDoorEventTrigger::Update(float ElapsedTime)
{
	if (m_IsActive && m_IsInteracted)
	{
		if (m_DoorAngle < 70.0f)
		{
			const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

			m_EventObjects[0]->Rotate(WorldUp, 50.0f * ElapsedTime);
			m_EventObjects[1]->Rotate(WorldUp, -50.0f * ElapsedTime);
			m_DoorAngle += 50.0f * ElapsedTime;
		}
	}
}

//=========================================================================================================================

CPowerDownEventTrigger::CPowerDownEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 40.0f;
}

void CPowerDownEventTrigger::Reset()
{
	CEventTrigger::Reset();

	const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

	CServer::m_Lights[0].m_IsActive = true;

	m_EventObjects[0]->Rotate(WorldUp, m_PanelAngle);
	m_IsOpened = false;
	m_PanelAngle = 0.0f;
}

void CPowerDownEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	// 0번 플레이어만이 이 트리거를 활성화시킬 수 있다.
	if (CallerIndex == 0)
	{
		m_IsInteracted = true;

		if (m_IsOpened)
		{
			CServer::m_Lights[0].m_IsActive = false;
		}
	}
}

void CPowerDownEventTrigger::Update(float ElapsedTime)
{
	if (m_IsActive && m_IsInteracted)
	{
		if (m_PanelAngle < 120.0f)
		{
			const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

			m_EventObjects[0]->Rotate(WorldUp, -70.0f * ElapsedTime);
			m_PanelAngle += 70.0f * ElapsedTime;
		}
		else
		{
			if (!m_IsOpened)
			{
				m_IsInteracted = false;
				m_IsOpened = true;
				m_PanelAngle = 120.0f;
			}
		}
	}
}

//=========================================================================================================================

CSirenEventTrigger::CSirenEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 40.0f;
}

void CSirenEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	// 1번 플레이어만이 이 트리거를 활성화시킬 수 있다.
	if (CallerIndex == 1)
	{
		m_IsInteracted = true;

		vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
		shared_ptr<CNavMesh> NavMesh{ CServer::m_NavMesh };

		const UINT GuardCount{ static_cast<UINT>(GameObjects[OBJECT_TYPE_NPC].size()) };
		const XMFLOAT3 CenterPosition{ 0.5f * (m_TriggerArea[0].x + m_TriggerArea[3].x), m_TriggerArea[0].y, 0.5f * (m_TriggerArea[0].z + m_TriggerArea[1].z) };

		for (UINT i = 3; i < GuardCount; ++i)
		{
			if (i == 3 || i == 5 || i == 6 || i == 9)
			{
				if (GameObjects[OBJECT_TYPE_NPC][i])
				{
					shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(GameObjects[OBJECT_TYPE_NPC][i]) };

					if (Guard->GetHealth() > 0)
					{
						// 사이렌을 작동시킬 경우 주변 범위에 있는 경찰들이 플레이어를 쫒기 시작한다.
						if (Guard->GetStateMachine()->IsInState(CGuardIdleState::GetInstance()) ||
							Guard->GetStateMachine()->IsInState(CGuardPatrolState::GetInstance()) ||
							Guard->GetStateMachine()->IsInState(CGuardReturnState::GetInstance()))
						{
							Guard->FindNavPath(NavMesh, CenterPosition, GameObjects);
							Guard->GetStateMachine()->ChangeState(CGuardAssembleState::GetInstance());
						}
					}
				}
			}
		}
	}
}

//=========================================================================================================================

COpenGateEventTrigger::COpenGateEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 70.0f;
}

void COpenGateEventTrigger::Reset()
{
	CEventTrigger::Reset();

	const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

	m_EventObjects[0]->Rotate(WorldUp, m_GateAngle);
	m_EventObjects[1]->Rotate(WorldUp, -m_GateAngle);
	m_UsedKeyIndices[0] = m_UsedKeyIndices[1] = false;
	m_GateAngle = 0.0f;
}

bool COpenGateEventTrigger::CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition)
{
	// 게이트 너머로 넘어가는 것을 계산한다.
	if (Math::LineIntersection(m_TriggerArea[0], m_TriggerArea[3], Position, NewPosition))
	{
		return false;
	}

	// 위 경우가 아니라면, 이동이 가능하다.
	return true;
}

void COpenGateEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(GameObjects[OBJECT_TYPE_PLAYER][CallerIndex]) };

	if (Player->HasKey() && !m_UsedKeyIndices[CallerIndex])
	{
		m_UsedKeyIndices[CallerIndex] = true;

		if (m_UsedKeyIndices[0] && m_UsedKeyIndices[1])
		{
			m_IsInteracted = true;
		}
	}
}

void COpenGateEventTrigger::Update(float ElapsedTime)
{
	if (m_IsActive && m_IsInteracted)
	{
		if (m_GateAngle < 120.0f)
		{
			const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

			m_EventObjects[0]->Rotate(WorldUp, -55.0f * ElapsedTime);
			m_EventObjects[1]->Rotate(WorldUp, 55.0f * ElapsedTime);
			m_GateAngle += 55.0f * ElapsedTime;
		}
		else
		{
			CServer::m_IsGameClear = true;
		}
	}
}

//=========================================================================================================================

CGetPistolEventTrigger::CGetPistolEventTrigger()
{
	m_ActiveFOV = 360.0f;
}

void CGetPistolEventTrigger::Reset()
{
	CEventTrigger::Reset();

	m_IsActive = false;
}

void CGetPistolEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	m_IsInteracted = true;

	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(GameObjects[OBJECT_TYPE_PLAYER][CallerIndex]) };

	if (!Player->HasPistol())
	{
		Player->ManagePistol(true);
	}

	Player->SwapWeapon(WEAPON_TYPE_PISTOL);
}

//=========================================================================================================================

CGetKeyEventTrigger::CGetKeyEventTrigger()
{
	m_ActiveFOV = 360.0f;
}

void CGetKeyEventTrigger::Reset()
{
	CEventTrigger::Reset();

	m_IsActive = false;
}

void CGetKeyEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	m_IsInteracted = true;

	static_pointer_cast<CPlayer>(CServer::m_GameObjects[OBJECT_TYPE_PLAYER][CallerIndex])->ManageKey(true);
}
