#include "stdafx.h"
#include "EventTriggers.h"

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

	//	// 배전함이 열려있다면
	//	if (IsOpened())
	//	{
	//		shared_ptr<CGameScene> GameScene{ static_pointer_cast<CGameScene>(CSceneManager::GetInstance()->GetCurrentScene()) };
	//		vector<LIGHT>& Lights{ GameScene->GetLights() };
	//		vector<vector<shared_ptr<CBilboardObject>>>& BilboardObjects{ GameScene->GetBilboardObjects() };

	//		// 감시탑의 조명을 끈다.
	//		Lights[1].m_IsActive = false;

	//		// 감시탑 차단 미션UI를 완료상태로 변경한다.
	//		BilboardObjects[BILBOARD_OBJECT_TYPE_UI][0]->SetCellIndex(0, 1);

	//		CSoundManager::GetInstance()->Play(SOUND_TYPE_POWER_DOWN, 0.65f);

	//		// 감시탑의 전원을 차단했다면, 현재 이벤트 트리거를 삭제한다.
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
	//					// 사이렌을 작동시킬 경우 주변 범위에 있는 경찰들이 플레이어를 쫒기 시작한다.
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

	//	// 사이렌을 작동시켰다면, 현재 이벤트 트리거를 삭제한다.
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
	//if (!IsInteracted())
	//{
	//	// 열쇠를 획득한 경우에만, 트리거를 활성화 시키도록 한다.
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

	//	// 권총을 획득한 경우, 권총으로 무기를 교체하고 UI 또한 주먹에서 권총으로 변경시킨다.
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

	//	// 권총을 획득했다면, 현재 이벤트 트리거를 삭제한다.
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

	//	// 열쇠 획득 애니메이션을 출력하도록 CKeyUIActivationState 상태로 전이한다.
	//	static_pointer_cast<CKeyUI>(BilboardObjects[BILBOARD_OBJECT_TYPE_UI][5])->GetStateMachine()->SetCurrentState(CKeyUIActivationState::GetInstance());

	//	// 열쇠 획득 미션UI를 완료상태로 변경한다.
	//	BilboardObjects[BILBOARD_OBJECT_TYPE_UI][0]->SetCellIndex(1, 5);

	//	// 열쇠를 획득했다면, 현재 이벤트 트리거를 삭제한다.
	//	DeleteThisEventTrigger();
	//}
}
