#include "stdafx.h"
#include "EventTriggers.h"

COpenDoorEventTrigger::COpenDoorEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 70.0f;
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

void COpenDoorEventTrigger::InteractEventTrigger()
{
	if (!IsInteracted())
	{
		CEventTrigger::InteractEventTrigger();
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
	//if (!IsInteracted())
	//{
	//	CEventTrigger::InteractEventTrigger();

	//	// �������� �����ִٸ�
	//	if (IsOpened())
	//	{
	//		shared_ptr<CGameScene> GameScene{ static_pointer_cast<CGameScene>(CSceneManager::GetInstance()->GetCurrentScene()) };
	//		vector<LIGHT>& Lights{ GameScene->GetLights() };
	//		vector<vector<shared_ptr<CBilboardObject>>>& BilboardObjects{ GameScene->GetBilboardObjects() };

	//		// ����ž�� ������ ����.
	//		Lights[1].m_IsActive = false;

	//		// ����ž ���� �̼�UI�� �Ϸ���·� �����Ѵ�.
	//		BilboardObjects[BILBOARD_OBJECT_TYPE_UI][0]->SetCellIndex(0, 1);

	//		CSoundManager::GetInstance()->Play(SOUND_TYPE_POWER_DOWN, 0.65f);

	//		// ����ž�� ������ �����ߴٸ�, ���� �̺�Ʈ Ʈ���Ÿ� �����Ѵ�.
	//		DeleteThisEventTrigger();
	//	}
	//	else
	//	{
	//		CSoundManager::GetInstance()->Play(SOUND_TYPE_OPEN_EP, 0.65f);
	//	}
	//}
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
	//if (!IsInteracted())
	//{
	//	CEventTrigger::InteractEventTrigger();

	//	shared_ptr<CGameScene> GameScene{ static_pointer_cast<CGameScene>(CSceneManager::GetInstance()->GetCurrentScene()) };
	//	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ GameScene->GetGameObjects() };
	//	shared_ptr<CNavMesh> NavMesh{ GameScene->GetNavMesh() };

	//	UINT GuardCount{ static_cast<UINT>(GameObjects[OBJECT_TYPE_NPC].size())};
	//	XMFLOAT3 CenterPosition{ (m_TriggerArea[0].x + m_TriggerArea[3].x) / 2.0f, m_TriggerArea[0].y, (m_TriggerArea[0].z + m_TriggerArea[1].z) / 2.0f };

	//	for (UINT i = 3; i < GuardCount; ++i)
	//	{
	//		if (i == 3 || i == 5 || i == 6 || i == 8 || i == 9)
	//		{
	//			if (GameObjects[OBJECT_TYPE_NPC][i])
	//			{
	//				shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(GameObjects[OBJECT_TYPE_NPC][i]) };

	//				if (Guard->GetHealth() > 0)
	//				{
	//					// ���̷��� �۵���ų ��� �ֺ� ������ �ִ� �������� �÷��̾ �i�� �����Ѵ�.
	//					if (Guard->GetStateMachine()->IsInState(CGuardIdleState::GetInstance()) ||
	//						Guard->GetStateMachine()->IsInState(CGuardPatrolState::GetInstance()) ||
	//						Guard->GetStateMachine()->IsInState(CGuardReturnState::GetInstance()))
	//					{
	//						Guard->FindNavPath(NavMesh, CenterPosition, GameObjects);
	//						Guard->GetStateMachine()->ChangeState(CGuardAssembleState::GetInstance());
	//					}
	//				}
	//			}
	//		}
	//	}

	//	CSoundManager::GetInstance()->Play(SOUND_TYPE_SIREN, 0.25f);

	//	// ���̷��� �۵����״ٸ�, ���� �̺�Ʈ Ʈ���Ÿ� �����Ѵ�.
	//	DeleteThisEventTrigger();
	//}
}

//=========================================================================================================================

COpenGateEventTrigger::COpenGateEventTrigger()
{
	m_IsActive = true;
	m_ActiveFOV = 70.0f;
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

void COpenGateEventTrigger::InteractEventTrigger()
{
	//if (!IsInteracted())
	//{
	//	// ���踦 ȹ���� ��쿡��, Ʈ���Ÿ� Ȱ��ȭ ��Ű���� �Ѵ�.
	//	vector<vector<shared_ptr<CBilboardObject>>>& BilboardObjects{ static_pointer_cast<CGameScene>(CSceneManager::GetInstance()->GetCurrentScene())->GetBilboardObjects() };

	//	if (static_pointer_cast<CKeyUI>(BilboardObjects[BILBOARD_OBJECT_TYPE_UI][5])->GetStateMachine()->IsInState(CKeyUIActivationState::GetInstance()))
	//	{
	//		CEventTrigger::InteractEventTrigger();
	//		CSoundManager::GetInstance()->Play(SOUND_TYPE_OPEN_GATE, 0.35f);
	//	}
	//}
}

//=========================================================================================================================

CGetPistolEventTrigger::CGetPistolEventTrigger()
{
	m_ActiveFOV = 360.0f;
}

void CGetPistolEventTrigger::InteractEventTrigger()
{
	//if (!IsInteracted())
	//{
	//	CEventTrigger::InteractEventTrigger();

	//	shared_ptr<CGameScene> GameScene{ static_pointer_cast<CGameScene>(CSceneManager::GetInstance()->GetCurrentScene()) };
	//	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ GameScene->GetGameObjects() };
	//	vector<vector<shared_ptr<CBilboardObject>>>& BilboardObjects{ GameScene->GetBilboardObjects() };

	//	// ������ ȹ���� ���, �������� ���⸦ ��ü�ϰ� UI ���� �ָԿ��� �������� �����Ų��.
	//	shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(GameObjects[OBJECT_TYPE_PLAYER].back()) };

	//	if (!Player->HasPistol())
	//	{
	//		Player->AcquirePistol();
	//	}

	//	Player->SwapWeapon(WEAPON_TYPE_PISTOL);

	//	BilboardObjects[BILBOARD_OBJECT_TYPE_UI][4]->SetActive(false); // 4: Punch UI
	//	BilboardObjects[BILBOARD_OBJECT_TYPE_UI][6]->SetActive(true);  // 6: Pistol UI
	//	BilboardObjects[BILBOARD_OBJECT_TYPE_UI][7]->SetActive(true);  // 7: Bullet UI
	//	BilboardObjects[BILBOARD_OBJECT_TYPE_UI][7]->SetVertexCount(5);

	//	// ������ ȹ���ߴٸ�, ���� �̺�Ʈ Ʈ���Ÿ� �����Ѵ�.
	//	DeleteThisEventTrigger();
	//}
}

//=========================================================================================================================

CGetKeyEventTrigger::CGetKeyEventTrigger()
{
	m_ActiveFOV = 360.0f;
}

void CGetKeyEventTrigger::InteractEventTrigger()
{
	//if (!IsInteracted())
	//{
	//	CEventTrigger::InteractEventTrigger();

	//	vector<vector<shared_ptr<CBilboardObject>>>& BilboardObjects{ static_pointer_cast<CGameScene>(CSceneManager::GetInstance()->GetCurrentScene())->GetBilboardObjects() };

	//	// ���� ȹ�� �ִϸ��̼��� ����ϵ��� CKeyUIActivationState ���·� �����Ѵ�.
	//	static_pointer_cast<CKeyUI>(BilboardObjects[BILBOARD_OBJECT_TYPE_UI][5])->GetStateMachine()->SetCurrentState(CKeyUIActivationState::GetInstance());

	//	// ���� ȹ�� �̼�UI�� �Ϸ���·� �����Ѵ�.
	//	BilboardObjects[BILBOARD_OBJECT_TYPE_UI][0]->SetCellIndex(1, 5);

	//	// ���踦 ȹ���ߴٸ�, ���� �̺�Ʈ Ʈ���Ÿ� �����Ѵ�.
	//	DeleteThisEventTrigger();
	//}
}
