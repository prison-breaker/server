#include "stdafx.h"
#include "Server.h"

CPlayerIdleState* CPlayerIdleState::GetInstance()
{
	static CPlayerIdleState Instance{};

	return &Instance;
}

void CPlayerIdleState::Enter(const shared_ptr<CPlayer>& Entity)
{
	Entity->SetSpeed(0.0f);
	Entity->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_IDLE);
}

void CPlayerIdleState::ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask)
{
	if (InputMask & INPUT_MASK_RMB)
	{
		if (Entity->IsEquippedPistol())
		{
			Entity->GetStateMachine()->ChangeState(CPlayerShootingState::GetInstance());
			return;
		}
	}

	if (InputMask & INPUT_MASK_LMB)
	{
		if (!Entity->IsEquippedPistol())
		{
			Entity->GetStateMachine()->ChangeState(CPlayerPunchingState::GetInstance());
			return;
		}
	}

	if (InputMask & INPUT_MASK_W || InputMask & INPUT_MASK_S || InputMask & INPUT_MASK_A || InputMask & INPUT_MASK_D)
	{
		if (InputMask & INPUT_MASK_SHIFT)
		{
			Entity->GetStateMachine()->ChangeState(CPlayerRunningState::GetInstance());
			return;
		}
		else
		{
			Entity->GetStateMachine()->ChangeState(CPlayerWalkingState::GetInstance());
			return;
		}
	}
}

void CPlayerIdleState::Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime)
{
	Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_LOOP);
}

void CPlayerIdleState::Exit(const shared_ptr<CPlayer>& Entity)
{

}

//=========================================================================================================================

CPlayerWalkingState* CPlayerWalkingState::GetInstance()
{
	static CPlayerWalkingState Instance{};

	return &Instance;
}

void CPlayerWalkingState::Enter(const shared_ptr<CPlayer>& Entity)
{
	Entity->SetSpeed(3.15f);
}

void CPlayerWalkingState::ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask)
{
	if (InputMask & INPUT_MASK_RMB)
	{
		if (Entity->IsEquippedPistol())
		{
			Entity->GetStateMachine()->ChangeState(CPlayerShootingState::GetInstance());
			return;
		}
	}

	if (InputMask & INPUT_MASK_LMB)
	{
		if (!Entity->IsEquippedPistol())
		{
			Entity->GetStateMachine()->ChangeState(CPlayerPunchingState::GetInstance());
			return;
		}
	}

	if (InputMask & INPUT_MASK_SHIFT)
	{
		if (InputMask & INPUT_MASK_W || InputMask & INPUT_MASK_S || InputMask & INPUT_MASK_A || InputMask & INPUT_MASK_D)
		{
			Entity->GetStateMachine()->ChangeState(CPlayerRunningState::GetInstance());
			return;
		}
	}

	XMFLOAT3 MovingDirection{};

	if (InputMask & INPUT_MASK_W)
	{
		MovingDirection = Vector3::Add(MovingDirection, Entity->GetLook());
	}

	if (InputMask & INPUT_MASK_S)
	{
		MovingDirection = Vector3::Add(MovingDirection, Vector3::Inverse(Entity->GetLook()));
	}

	if (InputMask & INPUT_MASK_A)
	{
		MovingDirection = Vector3::Add(MovingDirection, Vector3::Inverse(Entity->GetRight()));
	}

	if (InputMask & INPUT_MASK_D)
	{
		MovingDirection = Vector3::Add(MovingDirection, Entity->GetRight());
	}

	MovingDirection = Vector3::Normalize(MovingDirection);
	Entity->SetMovingDirection(MovingDirection);

	if (Vector3::IsZero(MovingDirection))
	{
		Entity->GetStateMachine()->ChangeState(CPlayerIdleState::GetInstance());
	}
	else if (Vector3::IsEqual(MovingDirection, Entity->GetRight()))
	{
		Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_WALK_RIGHT);
	}
	else if (Vector3::IsEqual(MovingDirection, Vector3::Inverse(Entity->GetRight())))
	{
		Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_WALK_LEFT);
	}
	else
	{
		Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_WALK_FORWARD_AND_BACK);
	}
}

void CPlayerWalkingState::Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime)
{
	Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_LOOP);
}

void CPlayerWalkingState::Exit(const shared_ptr<CPlayer>& Entity)
{

}

//=========================================================================================================================

CPlayerRunningState* CPlayerRunningState::GetInstance()
{
	static CPlayerRunningState Instance{};

	return &Instance;
}

void CPlayerRunningState::Enter(const shared_ptr<CPlayer>& Entity)
{

}

void CPlayerRunningState::ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask)
{
	if (InputMask & INPUT_MASK_RMB)
	{
		if (Entity->IsEquippedPistol())
		{
			Entity->GetStateMachine()->ChangeState(CPlayerShootingState::GetInstance());
			return;
		}
	}

	if (InputMask & INPUT_MASK_LMB)
	{
		if (!Entity->IsEquippedPistol())
		{
			Entity->GetStateMachine()->ChangeState(CPlayerPunchingState::GetInstance());
			return;
		}
	}

	if (!(InputMask & INPUT_MASK_SHIFT))
	{
		if (InputMask & INPUT_MASK_W || InputMask & INPUT_MASK_S || InputMask & INPUT_MASK_A || InputMask & INPUT_MASK_D)
		{
			Entity->GetStateMachine()->ChangeState(CPlayerWalkingState::GetInstance());
			return;
		}
	}

	XMFLOAT3 MovingDirection{};

	if (InputMask & INPUT_MASK_W)
	{
		MovingDirection = Vector3::Add(MovingDirection, Entity->GetLook());
	}

	if (InputMask & INPUT_MASK_S)
	{
		MovingDirection = Vector3::Add(MovingDirection, Vector3::Inverse(Entity->GetLook()));
	}

	if (InputMask & INPUT_MASK_A)
	{
		MovingDirection = Vector3::Add(MovingDirection, Vector3::Inverse(Entity->GetRight()));
	}

	if (InputMask & INPUT_MASK_D)
	{
		MovingDirection = Vector3::Add(MovingDirection, Entity->GetRight());
	}

	MovingDirection = Vector3::Normalize(MovingDirection);
	Entity->SetMovingDirection(MovingDirection);

	if (Vector3::IsZero(MovingDirection))
	{
		Entity->GetStateMachine()->ChangeState(CPlayerIdleState::GetInstance());
	}
	else if (Vector3::IsEqual(MovingDirection, Entity->GetRight()))
	{
		Entity->SetSpeed(12.6f);
		Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_RUN_RIGHT);
	}
	else if (Vector3::IsEqual(MovingDirection, Vector3::Inverse(Entity->GetRight())))
	{
		Entity->SetSpeed(12.6f);
		Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_RUN_LEFT);
	}
	else if (Vector3::Angle(MovingDirection, Entity->GetLook()) < 90.0f)
	{
		Entity->SetSpeed(12.6f);
		Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_RUN_FORWARD);
	}
	else
	{
		Entity->SetSpeed(3.15f);
		Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_WALK_FORWARD_AND_BACK);
	}
}

void CPlayerRunningState::Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime)
{
	Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_LOOP);
}

void CPlayerRunningState::Exit(const shared_ptr<CPlayer>& Entity)
{

}

//=========================================================================================================================

CPlayerPunchingState* CPlayerPunchingState::GetInstance()
{
	static CPlayerPunchingState Instance{};

	return &Instance;
}

void CPlayerPunchingState::Enter(const shared_ptr<CPlayer>& Entity)
{
	Entity->SetSpeed(0.0f);
	Entity->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_PUNCH);

	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

	for (const auto& GameObject : GameObjects[OBJECT_TYPE_NPC])
	{
		if (GameObject)
		{
			shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(GameObject) };

			if (Guard->GetHealth() > 0)
			{
				XMFLOAT3 ToGuard{ Vector3::Subtract(Guard->GetPosition(), Entity->GetPosition()) };

				if ((Vector3::Length(ToGuard) < 3.0f) && (Vector3::Angle(Entity->GetLook(), Vector3::Normalize(ToGuard)) < 80.0f))
				{
					// Ÿ���� ������ ��찡 �ƴ� ��쿡 �¾Ҵٸ�, �ڿ��� ���ݴ��� ����̹Ƿ�, Ÿ���� �������� �ʴ´�.
					// ��, Ÿ���� �־��� ��쿡��, Ÿ���� ���� ������� �����Ѵ�.
					if (Guard->GetTarget())
					{
						Guard->SetTarget(Entity);
					}

					Guard->GetStateMachine()->ChangeState(CGuardHitState::GetInstance());
				}
			}
		}
	}
}

void CPlayerPunchingState::ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CPlayerPunchingState::Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime)
{
	if (Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_ONCE))
	{
		Entity->GetStateMachine()->ChangeState(CPlayerIdleState::GetInstance());
	}
}

void CPlayerPunchingState::Exit(const shared_ptr<CPlayer>& Entity)
{

}

//=========================================================================================================================

CPlayerShootingState* CPlayerShootingState::GetInstance()
{
	static CPlayerShootingState Instance{};

	return &Instance;
}

void CPlayerShootingState::Enter(const shared_ptr<CPlayer>& Entity)
{
	Entity->SetSpeed(0.0f);
	Entity->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_PISTOL_IDLE);

}

void CPlayerShootingState::ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask)
{
	if ((InputMask & INPUT_MASK_LMB) && (InputMask & INPUT_MASK_RMB))
	{
		//vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

		//// �� �ִϸ��̼��� �ϴ� �����̰�, 
		//if (Entity->GetAnimationController()->GetAnimationClipType() == ANIMATION_CLIP_TYPE_PLAYER_PISTOL_IDLE)
		//{
		//	// ������ �Ѿ��� 1�� �̻��ִٸ�, ���� ���.
		//	if (BilboardObjects[BILBOARD_OBJECT_TYPE_UI][7]->GetVertexCount() > 0)
		//	{
		//		shared_ptr<CGameObject> NearestIntersectedRootObject{};
		//		shared_ptr<CGameObject> NearestIntersectedObject{};

		//		float NearestHitDistance{ FLT_MAX };
		//		float HitDistance{};
		//		XMFLOAT3 RayOrigin{ Entity->GetCamera()->GetPosition() };
		//		XMFLOAT3 RayDirection{ Entity->GetCamera()->GetLook() };

		//		for (UINT i = OBJECT_TYPE_NPC; i <= OBJECT_TYPE_STRUCTURE; ++i)
		//		{
		//			for (const auto& GameObject : GameObjects[i])
		//			{
		//				if (GameObject)
		//				{
		//					if (GameObject->IsActive())
		//					{
		//						if (i == OBJECT_TYPE_NPC)
		//						{
		//							shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(GameObject) };

		//							if (Guard->GetHealth() <= 0)
		//							{
		//								continue;
		//							}
		//						}

		//						// ���� �����ϱ� ������, ���� ��ȯ ����� ��ü���� ���Ž����־�� �Ѵ�.
		//						shared_ptr<CAnimationController> AnimationController{ GameObject->GetAnimationController() };

		//						if (AnimationController)
		//						{
		//							AnimationController->UpdateShaderVariables();
		//						}

		//						shared_ptr<CGameObject> IntersectedObject{ GameObject->PickObjectByRayIntersection(RayOrigin, RayDirection, HitDistance, FLT_MAX) };

		//						//if (IntersectedObject)
		//						//{
		//						//	tcout << TEXT("������ ���� ��ü�� : ") << IntersectedObject->GetName() << TEXT(" (�Ÿ� : ") << HitDistance << TEXT(")") << endl;
		//						//	tcout << TEXT("- �ش� ��ü�� ��ġ : ") << IntersectedObject->GetPosition().x << ", " << IntersectedObject->GetPosition().y << ", " << IntersectedObject->GetPosition().z << endl;
		//						//	tcout << TEXT("- �ش� ��ü �߽ɱ����� �Ÿ� : ") << Vector3::Length(Vector3::Subtract(IntersectedObject->GetPosition(), Entity->GetCamera()->GetPosition())) << endl;
		//						//}

		//						if (IntersectedObject && (HitDistance < NearestHitDistance))
		//						{
		//							NearestIntersectedRootObject = GameObject;
		//							NearestIntersectedObject = IntersectedObject;
		//							NearestHitDistance = HitDistance;
		//						}
		//					}
		//				}
		//			}
		//		}

		//		if (NearestIntersectedObject)
		//		{
		//			if (typeid(*NearestIntersectedRootObject) == typeid(CGuard))
		//			{
		//				shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(NearestIntersectedRootObject) };

		//				if (!Guard->GetStateMachine()->IsInState(CGuardHitState::GetInstance()) && !Guard->GetStateMachine()->IsInState(CGuardDyingState::GetInstance()))
		//				{
		//					Guard->SetTarget(Entity);
		//					Guard->GetStateMachine()->ChangeState(CGuardHitState::GetInstance());
		//				}
		//			}

		//			//tcout << TEXT("�� ���� ���� ������ ���� ��ü�� : ") << NearestIntersectedObject->GetName() << TEXT(" (�Ÿ� : ") << NearestHitDistance << TEXT(")") << endl << endl;
		//		}

		//		Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_SHOOT);

		//		// �Ѿ� UI�� �Ѿ� ������ �� �� ���ҽ�Ų��.
		//		BilboardObjects[BILBOARD_OBJECT_TYPE_UI][7]->SetVertexCount(BilboardObjects[BILBOARD_OBJECT_TYPE_UI][7]->GetVertexCount() - 1);

		//		CSoundManager::GetInstance()->Play(SOUND_TYPE_PISTOL_SHOT, 0.45f);
		//	}
		//	else
		//	{
		//		CSoundManager::GetInstance()->Play(SOUND_TYPE_PISTOL_EMPTY, 0.45f);
		//	}
		//}
	}
	else if (InputMask & INPUT_MASK_RMB)
	{
		//Entity->GetCamera()->IncreaseZoomFactor(ElapsedTime);
	}
	else
	{
		Entity->GetStateMachine()->ChangeState(CPlayerIdleState::GetInstance());
	}
}

void CPlayerShootingState::Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime)
{
	switch (Entity->GetAnimationController()->GetAnimationClipType())
	{
	case ANIMATION_CLIP_TYPE_PLAYER_PISTOL_IDLE:
		Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_LOOP);
		break;
	case ANIMATION_CLIP_TYPE_PLAYER_SHOOT:
		if (Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_ONCE))
		{
			Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_PISTOL_IDLE);
		}
		break;
	}
}

void CPlayerShootingState::Exit(const shared_ptr<CPlayer>& Entity)
{
	//Entity->GetCamera()->SetZoomIn(false);
}

//=========================================================================================================================

CPlayerDyingState* CPlayerDyingState::GetInstance()
{
	static CPlayerDyingState Instance{};

	return &Instance;
}

void CPlayerDyingState::Enter(const shared_ptr<CPlayer>& Entity)
{
	Entity->SetSpeed(0.0f);
	Entity->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_DIE);
}

void CPlayerDyingState::ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CPlayerDyingState::Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime)
{
	Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_ONCE);
}

void CPlayerDyingState::Exit(const shared_ptr<CPlayer>& Entity)
{

}