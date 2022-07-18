#include "stdafx.h"
#include "Server.h"

COpenDoorEventTrigger::COpenDoorEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 70.0f;
}

void COpenDoorEventTrigger::Reset()
{
	CEventTrigger::Reset();

	SetActive(true);

	const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

	m_EventObjects[0]->Rotate(WorldUp , -m_DoorAngle);
	m_EventObjects[1]->Rotate(WorldUp, m_DoorAngle);
	m_DoorAngle = 0.0f;
}

bool COpenDoorEventTrigger::CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition)
{
	// ���� ��� ������ ���� ���¿����� �� �ʸӷ� �� �� ����.
	if (m_DoorAngle < 70.0f)
	{
		// �� �ʸӷ� �Ѿ�� ���� ����Ѵ�.
		if (Math::LineIntersection(m_TriggerArea[0], m_TriggerArea[3], Position, NewPosition))
		{
			return false;
		}
	}

	// �� ��찡 �ƴ϶��, �̵��� �����ϴ�.
	return true;
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

	SetActive(true);

	const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

	CServer::m_Lights[0].m_IsActive = true;

	m_IsOpened = false;
	m_EventObjects[0]->Rotate(WorldUp, m_PanelAngle);
	m_PanelAngle = 0.0f;
}

void CPowerDownEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger(CallerIndex);

		// �������� �����ִٸ�
		if (IsOpened())
		{
			// ����ž�� ������ ����.
			CServer::m_Lights[0].m_IsActive = false;
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

void CSirenEventTrigger::Reset()
{
	CEventTrigger::Reset();

	SetActive(true);
}

void CSirenEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger(CallerIndex);

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
						// ���̷��� �۵���ų ��� �ֺ� ������ �ִ� �������� �÷��̾ �i�� �����Ѵ�.
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

	SetActive(true);

	const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

	m_EventObjects[0]->Rotate(WorldUp, m_GateAngle);
	m_EventObjects[1]->Rotate(WorldUp, -m_GateAngle);
	m_GateAngle = 0.0f;
}

bool COpenGateEventTrigger::CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition)
{
	// ����Ʈ�� ��� ������ ���� ���¿����� ����Ʈ �ʸӷ� �� �� ����.
	if (m_GateAngle < 120.0f)
	{
		// ����Ʈ �ʸӷ� �Ѿ�� ���� ����Ѵ�.
		if (Math::LineIntersection(m_TriggerArea[0], m_TriggerArea[3], Position, NewPosition))
		{
			return false;
		}
	}

	// �� ��찡 �ƴ϶��, �̵��� �����ϴ�.
	return true;
}

void COpenGateEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	if (!IsInteracted())
	{
		//vector<shared_ptr<CEventTrigger>>& EventTriggers{ CServer::m_EventTriggers };

		//if (EventTriggers[0]->IsInteracted() && EventTriggers[1]->IsInteracted())
		//{
		//	CEventTrigger::InteractEventTrigger(CallerIndex);
		//}
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

	SetActive(false);
}

void CGetPistolEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger(CallerIndex);

		vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

		// ������ ȹ���� ���, �������� ���⸦ ��ü�ϰ� UI ���� �ָԿ��� �������� �����Ų��.
		shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(GameObjects[OBJECT_TYPE_PLAYER][CallerIndex]) };

		if (!Player->HasPistol())
		{
			Player->ManagePistol(true);
		}

		Player->SwapWeapon(WEAPON_TYPE_PISTOL);
	}
}

//=========================================================================================================================

CGetKeyEventTrigger::CGetKeyEventTrigger()
{
	m_ActiveFOV = 360.0f;
}

void CGetKeyEventTrigger::Reset()
{
	CEventTrigger::Reset();

	SetActive(false);
}

void CGetKeyEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger(CallerIndex);
	}
}
