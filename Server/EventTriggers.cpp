#include "stdafx.h"
#include "Server.h"

COpenDoorEventTrigger::COpenDoorEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 70.0f;
}

bool COpenDoorEventTrigger::CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition)
{
	// 문이 모두 열리지 않은 상태에서는 문 너머로 갈 수 없다.
	if (m_DoorAngle < 70.0f)
	{
		// 문 너머로 넘어가는 것을 계산한다.
		if (Math::LineIntersection(m_TriggerArea[0], m_TriggerArea[3], Position, NewPosition))
		{
			return false;
		}
	}

	// 위 경우가 아니라면, 이동이 가능하다.
	return true;
}

void COpenDoorEventTrigger::InteractEventTrigger()
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger();

		if (CServer::m_CompletedTriggers & MSG_TYPE_TRIGGER_OPEN_PRISON_DOOR)
		{
			CServer::m_MsgType |= MSG_TYPE_TRIGGER_OPEN_GUARDPOST_DOOR;
			CServer::m_CompletedTriggers |= MSG_TYPE_TRIGGER_OPEN_GUARDPOST_DOOR;
		}
		else
		{
			CServer::m_MsgType |= MSG_TYPE_TRIGGER_OPEN_PRISON_DOOR;
			CServer::m_CompletedTriggers |= MSG_TYPE_TRIGGER_OPEN_PRISON_DOOR;
		}
	}
}

void COpenDoorEventTrigger::Update(float ElapsedTime)
{
	if (IsActive() && IsInteracted())
	{
		if (m_DoorAngle < 70.0f)
		{
			const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

			m_EventObjects[0]->Rotate(WorldUp, 50.0f * ElapsedTime);
			m_EventObjects[1]->Rotate(WorldUp, -50.0f * ElapsedTime);
			m_DoorAngle += 50.0f * ElapsedTime;
		}
		else
		{
			// 문이 모두 열렸다면, 현재 이벤트 트리거를 삭제한다.
			DeleteThisEventTrigger();
		}
	}
}

//=========================================================================================================================

CPowerDownEventTrigger::CPowerDownEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 40.0f;
}

void CPowerDownEventTrigger::InteractEventTrigger()
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger();

		// 배전함이 열려있다면
		if (IsOpened())
		{
			CServer::m_MsgType |= MSG_TYPE_TRIGGER_POWER_DOWN_TOWER;
			CServer::m_CompletedTriggers |= MSG_TYPE_TRIGGER_POWER_DOWN_TOWER;

			// 감시탑의 조명을 끈다.
			CServer::m_Lights[0].m_IsActive = false;

			// 감시탑의 전원을 차단했다면, 현재 이벤트 트리거를 삭제한다.
			DeleteThisEventTrigger();
		}
		else
		{
			CServer::m_MsgType |= MSG_TYPE_TRIGGER_OPEN_ELEC_PANEL;
			CServer::m_CompletedTriggers |= MSG_TYPE_TRIGGER_OPEN_ELEC_PANEL;
		}
	}
}

void CPowerDownEventTrigger::Update(float ElapsedTime)
{
	if (IsActive() && IsInteracted())
	{
		if (m_PanelAngle < 120.0f)
		{
			const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

			m_EventObjects[0]->Rotate(WorldUp, -70.0f * ElapsedTime);
			m_PanelAngle += 70.0f * ElapsedTime;
		}
		else
		{
			if (!IsOpened())
			{
				m_IsOpened = true;
				m_PanelAngle = 120.0f;

				SetInteracted(false);
			}
		}
	}
}

bool CPowerDownEventTrigger::IsOpened() const
{
	return m_IsOpened;
}

//=========================================================================================================================

CSirenEventTrigger::CSirenEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 40.0f;
}

void CSirenEventTrigger::InteractEventTrigger()
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger();

		CServer::m_MsgType |= MSG_TYPE_TRIGGER_SIREN;
		CServer::m_CompletedTriggers |= MSG_TYPE_TRIGGER_SIREN;

		vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
		shared_ptr<CNavMesh> NavMesh{ CServer::m_NavMesh };

		UINT GuardCount{ static_cast<UINT>(GameObjects[OBJECT_TYPE_NPC].size())};
		XMFLOAT3 CenterPosition{ (m_TriggerArea[0].x + m_TriggerArea[3].x) / 2.0f, m_TriggerArea[0].y, (m_TriggerArea[0].z + m_TriggerArea[1].z) / 2.0f };

		for (UINT i = 3; i < GuardCount; ++i)
		{
			if (i == 3 || i == 5 || i == 6 || i == 8 || i == 9)
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

		// 사이렌을 작동시켰다면, 현재 이벤트 트리거를 삭제한다.
		DeleteThisEventTrigger();
	}
}

//=========================================================================================================================

COpenGateEventTrigger::COpenGateEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 70.0f;
}

bool COpenGateEventTrigger::CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition)
{
	// 게이트가 모두 열리지 않은 상태에서는 게이트 너머로 갈 수 없다.
	if (m_GateAngle < 120.0f)
	{
		// 게이트 너머로 넘어가는 것을 계산한다.
		if (Math::LineIntersection(m_TriggerArea[0], m_TriggerArea[3], Position, NewPosition))
		{
			return false;
		}
	}

	// 위 경우가 아니라면, 이동이 가능하다.
	return true;
}

void COpenGateEventTrigger::InteractEventTrigger()
{
	if (!IsInteracted())
	{
		if ((CServer::m_CompletedTriggers & MSG_TYPE_TRIGGER_POWER_DOWN_TOWER) && (CServer::m_CompletedTriggers & MSG_TYPE_TRIGGER_SIREN))
		{
			CEventTrigger::InteractEventTrigger();

			CServer::m_MsgType |= MSG_TYPE_TRIGGER_OPEN_GATE;
			CServer::m_CompletedTriggers |= MSG_TYPE_TRIGGER_OPEN_GATE;
		}
	}
}

void COpenGateEventTrigger::Update(float ElapsedTime)
{
	if (IsActive() && IsInteracted())
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
			// 문이 모두 열렸다면, 현재 이벤트 트리거를 삭제한다.
			DeleteThisEventTrigger();
		}
	}
}

//=========================================================================================================================

CGetPistolEventTrigger::CGetPistolEventTrigger()
{
	m_ActiveFOV = 360.0f;
}

void CGetPistolEventTrigger::InteractEventTrigger()
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger();

		vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

		// 권총을 획득한 경우, 권총으로 무기를 교체하고 UI 또한 주먹에서 권총으로 변경시킨다.
		shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(GameObjects[OBJECT_TYPE_PLAYER].back()) };

		if (!Player->HasPistol())
		{
			Player->AcquirePistol();
		}

		Player->SwapWeapon(WEAPON_TYPE_PISTOL);

		// 권총을 획득했다면, 현재 이벤트 트리거를 삭제한다.
		DeleteThisEventTrigger();
	}
}

//=========================================================================================================================

CGetKeyEventTrigger::CGetKeyEventTrigger()
{
	m_ActiveFOV = 360.0f;
}

void CGetKeyEventTrigger::InteractEventTrigger()
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger();

		// 열쇠를 획득했다면, 현재 이벤트 트리거를 삭제한다.
		DeleteThisEventTrigger();
	}
}
