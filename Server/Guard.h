#pragma once
#include "GameObject.h"
#include "State_Guard.h"

class CEventTrigger;

class CGuard : public CGameObject
{
private:
	UINT							  m_Health{ 100 };

	float							  m_Speed{};
	XMFLOAT3						  m_MovingDirection{};

	shared_ptr<CStateMachine<CGuard>> m_StateMachine{};

	bool							  m_RecentTransition{};

	float							  m_ElapsedTime{};
	const float						  m_ToIdleEntryTime{ Random::Range(5.0f, 7.0f) };
	const float						  m_UpdateTargetTime{ 3.0f };

	shared_ptr<CGameObject>			  m_Target{};
	shared_ptr<CEventTrigger>		  m_EventTrigger{};

	vector<XMFLOAT3>                  m_NavPath{};
	vector<XMFLOAT3>				  m_PatrolNavPath{};
	UINT							  m_PatrolIndex{};

public:
	CGuard() = default;
	virtual ~CGuard() = default;

	virtual void Initialize();

	virtual void Animate(float ElapsedTime);

	void SetHealth(UINT Health);
	UINT GetHealth() const;

	void SetSpeed(float Speed);
	float GetSpeed() const;

	void SetMovingDirection(const XMFLOAT3& MovingDirection);
	const XMFLOAT3& GetMovingDirection() const;

	shared_ptr<CStateMachine<CGuard>> GetStateMachine() const;

	void SetRecentTransition(bool RecentTransition);
	bool GetRecentTransition() const;

	void SetElapsedTime(float ElapsedTime);
	float GetElapsedTime() const;

	float GetToIdleEntryTime() const;
	float GetUpdateTargetTime() const;

	void SetTarget(const shared_ptr<CGameObject>& Target);
	shared_ptr<CGameObject> GetTarget() const;

	void SetEventTrigger(const shared_ptr<CEventTrigger>& EventTrigger);
	shared_ptr<CEventTrigger> GetEventTrigger();

	vector<XMFLOAT3>& GetNavPath();
	vector<XMFLOAT3>& GetPatrolNavPath();

	UINT GetPatrolIndex() const;

	shared_ptr<CGameObject> IsFoundPlayer(const vector<vector<shared_ptr<CGameObject>>>& GameObjects) const;

	void FindNavPath(const shared_ptr<CNavMesh>& NavMesh, const XMFLOAT3& TargetPosition, const vector<vector<shared_ptr<CGameObject>>>& GameObjects);
	void FindRayCastingNavPath(const vector<vector<shared_ptr<CGameObject>>>& GameObjects);

	void FindPatrolNavPath(const shared_ptr<CNavMesh>& NavMesh, const XMFLOAT3& TargetPosition);

	void MoveToNavPath(float ElapsedTime);
	void Patrol(float ElapsedTime);

	void GenerateTrigger();
};
