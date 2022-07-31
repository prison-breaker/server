#include "stdafx.h"
#include "State_Guard.h"
#include "Server.h"
#include "Player.h"
#include "State_Player.h"
#include "Guard.h"
#include "StateMachine.h"
#include "AnimationController.h"

CGuardIdleState* CGuardIdleState::GetInstance()
{
	static CGuardIdleState Instance{};

	return &Instance;
}

void CGuardIdleState::Enter(const shared_ptr<CGuard>& Entity)
{
	Entity->SetElapsedTime(0.0f);
	Entity->SetRecentTransition(false);
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_NPC_IDLE);
	Entity->SetSpeed(0.0f);
}

void CGuardIdleState::ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CGuardIdleState::Update(const shared_ptr<CGuard>& Entity, float ElapsedTime)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CGameObject> NearestPlayer{ Entity->IsFoundPlayer(GameObjects) };

	Entity->SetTarget(NearestPlayer);

	if (NearestPlayer)
	{
		Entity->GetStateMachine()->ChangeState(CGuardChaseState::GetInstance());
	}
	else
	{
		// �ִϸ��̼��� ������ Patrol ���·� �����Ѵ�.
		if (Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_ONCE))
		{
			Entity->GetStateMachine()->ChangeState(CGuardPatrolState::GetInstance());
		}
	}
}

void CGuardIdleState::Exit(const shared_ptr<CGuard>& Entity)
{

}

//=========================================================================================================================

CGuardPatrolState* CGuardPatrolState::GetInstance()
{
	static CGuardPatrolState Instance{};

	return &Instance;
}

void CGuardPatrolState::Enter(const shared_ptr<CGuard>& Entity)
{
	XMFLOAT3 Direction{ Vector3::Normalize(Vector3::Subtract(Entity->GetPatrolNavPath()[Entity->GetPatrolIndex()], Entity->GetPosition())) };

	Entity->UpdateLocalCoord(Direction);
	Entity->SetElapsedTime(0.0f);
	Entity->SetRecentTransition(false);
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_NPC_WALK_FORWARD);
	Entity->SetSpeed(5.0f);
}

void CGuardPatrolState::ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CGuardPatrolState::Update(const shared_ptr<CGuard>& Entity, float ElapsedTime)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CGameObject> NearestPlayer{ Entity->IsFoundPlayer(GameObjects) };

	Entity->SetTarget(NearestPlayer);

	if (NearestPlayer)
	{
		Entity->GetStateMachine()->ChangeState(CGuardChaseState::GetInstance());
	}
	else
	{
		// ToIdleEntryTime�� �ȴٸ� IdleState�� �����Ѵ�.
		Entity->SetElapsedTime(Entity->GetElapsedTime() + ElapsedTime);

		if (Entity->GetElapsedTime() > Entity->GetToIdleEntryTime())
		{
			Entity->GetStateMachine()->ChangeState(CGuardIdleState::GetInstance());
		}
		else
		{
			Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_LOOP);
			Entity->Patrol(ElapsedTime);
		}
	}
}

void CGuardPatrolState::Exit(const shared_ptr<CGuard>& Entity)
{

}

//=========================================================================================================================

CGuardChaseState* CGuardChaseState::GetInstance()
{
	static CGuardChaseState Instance{};

	return &Instance;
}

void CGuardChaseState::Enter(const shared_ptr<CGuard>& Entity)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CNavMesh> NavMesh{ CServer::m_NavMesh };
	shared_ptr<CGameObject> Target{ Entity->GetTarget() };

	// �� State���� Chase ���·� �Ѿ���� ���ؼ��� Target�� �����Ǿ�� �ϱ� ������ Target�� ���� ������ Null üũ�� �ʿ����.
	Entity->FindNavPath(NavMesh, Target->GetPosition(), GameObjects);

	XMFLOAT3 Direction{ Vector3::Normalize(Vector3::Subtract(Entity->GetNavPath().back(), Entity->GetPosition())) };

	Entity->UpdateLocalCoord(Direction);
	Entity->SetElapsedTime(0.0f);
	Entity->SetRecentTransition(false);
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_NPC_RUN_FORWARD);
	Entity->SetSpeed(13.0f);
}

void CGuardChaseState::ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CGuardChaseState::Update(const shared_ptr<CGuard>& Entity, float ElapsedTime)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CGameObject> NearestPlayer{ Entity->IsFoundPlayer(GameObjects) };

	if (NearestPlayer)
	{
		Entity->SetTarget(NearestPlayer);
	}

	shared_ptr<CGameObject> Target{ Entity->GetTarget() };

	if (!NearestPlayer && Entity->GetRecentTransition())
	{
		Entity->SetTarget(NearestPlayer);

		// �÷��̾ �þ߰����� ������ �ʴ´ٸ� ReturnState�� �����Ͽ� ���� �����ϴ� ��ġ�� ���ư���.
		Entity->GetStateMachine()->ChangeState(CGuardReturnState::GetInstance());
		return;
	}
	else if (Target)
	{
		XMFLOAT3 Direction{};

		if (Math::Distance(Target->GetPosition(), Entity->GetPosition()) < 10.0f)
		{
			// �÷��̾�� �����Ÿ� ���ϰ� �Ǹ� RayCasting�� �Ͽ� ���� �ľ��� �� ���� ���.
			Direction = Vector3::Normalize(Vector3::Subtract(Target->GetPosition(), Entity->GetPosition()));

			float NearestHitDistance{ FLT_MAX };
			float HitDistance{};
			bool IsHit{};

			for (const auto& GameObject : GameObjects[OBJECT_TYPE_STRUCTURE])
			{
				if (GameObject)
				{
					XMFLOAT3 RayOrigin{ Entity->GetPosition() };
					RayOrigin.y = 5.0f;

					shared_ptr<CGameObject> IntersectedObject{ GameObject->PickObjectByRayIntersection(RayOrigin, Direction, HitDistance, 10.0f, false) };

					if (IntersectedObject && HitDistance < 10.0f)
					{
						IsHit = true;
						break;
					}
				}
			}

			if (!IsHit)
			{
				Entity->UpdateLocalCoord(Direction);
				Entity->GetStateMachine()->ChangeState(CGuardShootingState::GetInstance());
				return;
			}
		}

		if (Entity->GetElapsedTime() > Entity->GetUpdateTargetTime() || Entity->GetNavPath().empty())
		{
			shared_ptr<CNavMesh> NavMesh{ CServer::m_NavMesh };

			// 3�ʿ� �ѹ��� Ȥ�� NavPath�� ����� ��� NavPath�� �����Ѵ�.
			Entity->FindNavPath(NavMesh, Target->GetPosition(), GameObjects);

			Direction = Vector3::Normalize(Vector3::Subtract(Entity->GetNavPath().back(), Entity->GetPosition()));

			Entity->UpdateLocalCoord(Direction);
			Entity->SetElapsedTime(0.0f);
		}
	}

	Entity->SetElapsedTime(Entity->GetElapsedTime() + ElapsedTime);
	Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_LOOP);
	Entity->MoveToNavPath(ElapsedTime);
}

void CGuardChaseState::Exit(const shared_ptr<CGuard>& Entity)
{

}

//=========================================================================================================================

CGuardReturnState* CGuardReturnState::GetInstance()
{
	static CGuardReturnState Instance{};

	return &Instance;
}

void CGuardReturnState::Enter(const shared_ptr<CGuard>& Entity)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CNavMesh> NavMesh{ CServer::m_NavMesh };

	Entity->FindNavPath(NavMesh, Entity->GetPatrolNavPath()[Entity->GetPatrolIndex()], GameObjects);

	XMFLOAT3 Direction{ Vector3::Normalize(Vector3::Subtract(Entity->GetNavPath().back(), Entity->GetPosition())) };

	Entity->UpdateLocalCoord(Direction);
	Entity->SetElapsedTime(0.0f);
	Entity->SetRecentTransition(false);
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_NPC_WALK_FORWARD);
	Entity->SetSpeed(5.0f);
}

void CGuardReturnState::ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CGuardReturnState::Update(const shared_ptr<CGuard>& Entity, float ElapsedTime)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CGameObject> NearestPlayer{ Entity->IsFoundPlayer(GameObjects) };

	Entity->SetTarget(NearestPlayer);

	if (NearestPlayer)
	{
		Entity->GetStateMachine()->ChangeState(CGuardChaseState::GetInstance());
	}
	else
	{
		Entity->MoveToNavPath(ElapsedTime);
		Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_LOOP);

		// ���� ���������� ���ư��ٸ� PatrolState�� �����Ѵ�.
		if (Entity->GetNavPath().empty())
		{
			Entity->GetStateMachine()->ChangeState(CGuardPatrolState::GetInstance());
		}
	}
}

void CGuardReturnState::Exit(const shared_ptr<CGuard>& Entity)
{

}

// ========================================================================================================================

CGuardAssembleState* CGuardAssembleState::GetInstance()
{
	static CGuardAssembleState Instance{};

	return &Instance;
}

void CGuardAssembleState::Enter(const shared_ptr<CGuard>& Entity)
{
	XMFLOAT3 Direction{ Vector3::Normalize(Vector3::Subtract(Entity->GetNavPath().back(), Entity->GetPosition())) };

	Entity->UpdateLocalCoord(Direction);
	Entity->SetElapsedTime(0.0f);
	Entity->SetRecentTransition(false);
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_NPC_RUN_FORWARD);
	Entity->SetSpeed(13.0f);
}

void CGuardAssembleState::ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CGuardAssembleState::Update(const shared_ptr<CGuard>& Entity, float ElapsedTime)
{
	vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
	shared_ptr<CGameObject> NearestPlayer{ Entity->IsFoundPlayer(GameObjects) };

	Entity->SetTarget(NearestPlayer);

	if (NearestPlayer)
	{
		Entity->GetStateMachine()->ChangeState(CGuardChaseState::GetInstance());
	}
	else
	{
		Entity->MoveToNavPath(ElapsedTime);
		Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_LOOP);

		if (Entity->GetNavPath().empty())
		{
			Entity->GetStateMachine()->ChangeState(CGuardReturnState::GetInstance());
		}
	}
}

void CGuardAssembleState::Exit(const shared_ptr<CGuard>& Entity)
{

}

//=========================================================================================================================

CGuardShootingState* CGuardShootingState::GetInstance()
{
	static CGuardShootingState Instance{};

	return &Instance;
}

void CGuardShootingState::Enter(const shared_ptr<CGuard>& Entity)
{
	Entity->SetElapsedTime(0.0f);
	Entity->SetRecentTransition(false);
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_NPC_SHOOT);
	Entity->GetAnimationController()->SetKeyFrameIndex(0);
	Entity->SetSpeed(0.0f);

	if (!CServer::m_InvincibleMode)
	{
		shared_ptr<CPlayer> Player{ static_pointer_cast<CPlayer>(Entity->GetTarget()) };

		Player->SetHealth(Player->GetHealth() - 10);

		if (Player->GetHealth() <= 0)
		{
			Player->GetStateMachine()->ChangeState(CPlayerDyingState::GetInstance());
		}

		CServer::m_MsgType |= MSG_TYPE_GUARD_ATTACK;
		CServer::m_GuardAttackData.m_TargetIndices[Entity->GetID()] = Player->GetID();
	}
}

void CGuardShootingState::ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CGuardShootingState::Update(const shared_ptr<CGuard>& Entity, float ElapsedTime)
{
	if (Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_ONCE))
	{
		vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };
		shared_ptr<CPlayer> Target{ static_pointer_cast<CPlayer>(Entity->GetTarget()) };

		if (Target->GetHealth() > 0)
		{
			if (Math::Distance(Target->GetPosition(), Entity->GetPosition()) < 10.0f)
			{
				// �÷��̾�� �����Ÿ� ���ϰ� �Ǹ� RayCasting�� �Ͽ� ���� �ľ��� �� ���� ���.
				XMFLOAT3 Direction{ Vector3::Normalize(Vector3::Subtract(Target->GetPosition(), Entity->GetPosition())) };

				float NearestHitDistance{ FLT_MAX };
				float HitDistance{};
				bool IsHit{};

				for (const auto& GameObject : GameObjects[OBJECT_TYPE_STRUCTURE])
				{
					if (GameObject)
					{
						XMFLOAT3 RayOrigin{ Entity->GetPosition() };
						RayOrigin.y = 5.0f;

						shared_ptr<CGameObject> IntersectedObject{ GameObject->PickObjectByRayIntersection(RayOrigin, Direction, HitDistance, 10.0f, false) };

						if (IntersectedObject && HitDistance < 10.0f)
						{
							IsHit = true;
							break;
						}
					}
				}

				if (!IsHit)
				{
					// RayCasting ���� �� �¾Ҵٸ� ������ �ٽ� ������ �� ���� �ִϸ��̼��� �ϵ��� ��Ʈ�ѷ��� �ε����� 0���� �����.
					Entity->UpdateLocalCoord(Direction);
					Entity->GetStateMachine()->GetCurrentState()->Enter(Entity);
					return;
				}
			}
		}

		shared_ptr<CGameObject> NearestPlayer{ Entity->IsFoundPlayer(GameObjects) };

		Entity->SetTarget(NearestPlayer);

		if (NearestPlayer)
		{
			Entity->GetStateMachine()->ChangeState(CGuardChaseState::GetInstance());
		}
		else
		{
			Entity->GetStateMachine()->ChangeState(CGuardReturnState::GetInstance());
		}
	}
}

void CGuardShootingState::Exit(const shared_ptr<CGuard>& Entity)
{

}

//=========================================================================================================================

CGuardHitState* CGuardHitState::GetInstance()
{
	static CGuardHitState Instance{};

	return &Instance;
}

void CGuardHitState::Enter(const shared_ptr<CGuard>& Entity)
{
	if (Entity->GetHealth() <= 0)
	{
		Entity->GetStateMachine()->ChangeState(CGuardDyingState::GetInstance());

		return;
	}

	Entity->SetElapsedTime(0.0f);
	Entity->SetRecentTransition(false);
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_NPC_HIT);
	Entity->SetSpeed(0.0f);
}

void CGuardHitState::ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CGuardHitState::Update(const shared_ptr<CGuard>& Entity, float ElapsedTime)
{
	if (Entity->GetHealth() <= 0)
	{
		Entity->GetStateMachine()->ChangeState(CGuardDyingState::GetInstance());
		return;
	}

	if (Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_ONCE))
	{
		// �̶�, Target�� Player�� PunchingState�� ShootingState���� Set �Ǽ� HitState�� ������ ������ ������ Set�� �ʿ� ����.
		Entity->GetStateMachine()->ChangeState(CGuardChaseState::GetInstance());
	}
}

void CGuardHitState::Exit(const shared_ptr<CGuard>& Entity)
{

}

//=========================================================================================================================

CGuardDyingState* CGuardDyingState::GetInstance()
{
	static CGuardDyingState Instance{};

	return &Instance;
}

void CGuardDyingState::Enter(const shared_ptr<CGuard>& Entity)
{
	Entity->SetElapsedTime(0.0f);
	Entity->SetRecentTransition(false);
	Entity->GetAnimationController()->SetAnimationClipType(ANIMATION_CLIP_TYPE_NPC_DIE);
	Entity->SetSpeed(0.0f);
	Entity->GenerateTrigger();
}

void CGuardDyingState::ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask)
{

}

void CGuardDyingState::Update(const shared_ptr<CGuard>& Entity, float ElapsedTime)
{
	// �������� ����ϸ� ������Ʈ�� ��Ȱ��ȭ�Ѵ�.
	if (Entity->GetAnimationController()->UpdateAnimationClip(ANIMATION_TYPE_ONCE))
	{
		Entity->SetElapsedTime(Entity->GetElapsedTime() + ElapsedTime);

		if (Entity->GetElapsedTime() > 10.0f)
		{
			Entity->SetActive(false);
		}
	}
}

void CGuardDyingState::Exit(const shared_ptr<CGuard>& Entity)
{

}
