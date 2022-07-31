#include "stdafx.h"
#include "State_Player.h"
#include "Server.h"
#include "Player.h"
#include "Guard.h"
#include "State_Guard.h"
#include "StateMachine.h"
#include "AnimationController.h"

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
				if (!Guard->GetStateMachine()->IsInState(CGuardHitState::GetInstance()))
				{
					XMFLOAT3 ToGuard{ Vector3::Subtract(Guard->GetPosition(), Entity->GetPosition()) };

					if ((Vector3::Length(ToGuard) < 3.0f) && (Vector3::Angle(Entity->GetLook(), Vector3::Normalize(ToGuard)) < 80.0f))
					{
						//교도관의 룩벡터와 플레이어의 룩벡터의 방향이 비슷하다면 뒷통수 펀치 판정
						if (Vector3::Angle(Guard->GetLook(), Entity->GetLook()) < 40.0f)
						{
							Guard->SetHealth(0);
							Guard->SetTarget(nullptr);
						}
						else
						{
							Guard->SetHealth(Guard->GetHealth() - 34);
							Guard->SetTarget(Entity);
						}

						Guard->GetStateMachine()->ChangeState(CGuardHitState::GetInstance());

						CServer::m_MsgType |= MSG_TYPE_PLAYER_ATTACK;
						CServer::m_PlayerAttackData.m_TargetIndices[Entity->GetID()] = Guard->GetID();

						break;
					}
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
	// 줌 애니메이션을 하고 있는 상태에서, 좌클릭을 입력받았다면, 총을 발사한다.
	// 총알에 대한 검사는 클라이언트에서 수행한 후 InputMask가 채워지기 때문에 서버에서 총알에 대한 검사는 수행할 필요 없다.
	if ((InputMask & INPUT_MASK_LMB) && (InputMask & INPUT_MASK_RMB))
	{
		if (Entity->GetAnimationController()->GetAnimationClipType() == ANIMATION_CLIP_TYPE_PLAYER_PISTOL_IDLE)
		{
			vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

			shared_ptr<CGameObject> NearestIntersectedRootObject{};
			shared_ptr<CGameObject> NearestIntersectedObject{};

			float NearestHitDistance{ FLT_MAX };
			float HitDistance{};
			XMFLOAT3 RayOrigin{ Entity->GetCameraData().m_CameraPosition };
			XMFLOAT3 RayDirection{ Entity->GetCameraData().m_CameraDirection };

			for (UINT i = OBJECT_TYPE_NPC; i <= OBJECT_TYPE_STRUCTURE; ++i)
			{
				for (const auto& GameObject : GameObjects[i])
				{
					if (GameObject)
					{
						if (GameObject->IsActive())
						{
							if (i == OBJECT_TYPE_NPC)
							{
								shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(GameObject) };

								if (Guard->GetHealth() <= 0)
								{
									continue;
								}
							}

							// 모델을 공유하기 때문에, 월드 변환 행렬을 객체마다 갱신시켜주어야 한다.
							shared_ptr<CAnimationController> AnimationController{ GameObject->GetAnimationController() };

							if (AnimationController)
							{
								AnimationController->UpdateShaderVariables();
							}

							shared_ptr<CGameObject> IntersectedObject{ GameObject->PickObjectByRayIntersection(RayOrigin, RayDirection, HitDistance, FLT_MAX, false) };

							//if (IntersectedObject)
							//{
							//	tcout << TEXT("광선을 맞은 객체명 : ") << IntersectedObject->GetName() << TEXT(" (거리 : ") << HitDistance << TEXT(")") << endl;
							//	tcout << TEXT("- 해당 객체의 위치 : ") << IntersectedObject->GetPosition().x << ", " << IntersectedObject->GetPosition().y << ", " << IntersectedObject->GetPosition().z << endl;
							//	tcout << TEXT("- 해당 객체 중심까지의 거리 : ") << Vector3::Length(Vector3::Subtract(IntersectedObject->GetPosition(), Entity->GetCamera()->GetPosition())) << endl;
							//}

							if (IntersectedObject && (HitDistance < NearestHitDistance))
							{
								NearestIntersectedRootObject = GameObject;
								NearestIntersectedObject = IntersectedObject;
								NearestHitDistance = HitDistance;
							}
						}
					}
				}
			}

			if (NearestIntersectedObject)
			{
				if (typeid(*NearestIntersectedRootObject) == typeid(CGuard))
				{
					shared_ptr<CGuard> Guard{ static_pointer_cast<CGuard>(NearestIntersectedRootObject) };

					if (Guard->GetHealth() > 0)
					{
						if (!Guard->GetStateMachine()->IsInState(CGuardHitState::GetInstance()))
						{
							tstring HitFrameName{ NearestIntersectedObject->GetName() };

							if (HitFrameName == "hat" || HitFrameName == "head_1" || HitFrameName == "head_2")
							{
								Guard->SetHealth(0);
								Guard->SetTarget(nullptr);
							}
							else
							{
								Guard->SetHealth(Guard->GetHealth() - 50);
								Guard->SetTarget(Entity);
							}

							Guard->GetStateMachine()->ChangeState(CGuardHitState::GetInstance());

							CServer::m_MsgType |= MSG_TYPE_PLAYER_ATTACK;
							CServer::m_PlayerAttackData.m_TargetIndices[Entity->GetID()] = Guard->GetID();
						}
					}
				}

				//tcout << TEXT("★ 가장 먼저 광선을 맞은 객체명 : ") << NearestIntersectedObject->GetName() << TEXT(" (거리 : ") << NearestHitDistance << TEXT(")") << endl << endl;
			}

			Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_PLAYER_SHOOT);
		}
	}
	else if (InputMask & INPUT_MASK_RMB)
	{
		// 조준 애니메이션을 유지한다.
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
