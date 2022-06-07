#pragma once
#include "State.h"

class CGuard;

class CGuardIdleState : public CState<CGuard>
{
private:
	CGuardIdleState() = default;
	virtual ~CGuardIdleState() = default;

public:
	static CGuardIdleState* GetInstance();

	virtual void Enter(const shared_ptr<CGuard>& Entity);
	virtual void ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CGuard>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CGuard>& Entity);
};

//=========================================================================================================================

class CGuardPatrolState : public CState<CGuard>
{
private:
	CGuardPatrolState() = default;
	virtual ~CGuardPatrolState() = default;

public:
	static CGuardPatrolState* GetInstance();

	virtual void Enter(const shared_ptr<CGuard>& Entity);
	virtual void ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CGuard>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CGuard>& Entity);
};

//=========================================================================================================================

class CGuardChaseState : public CState<CGuard>
{
private:
	CGuardChaseState() = default;
	virtual ~CGuardChaseState() = default;

public:
	static CGuardChaseState* GetInstance();

	virtual void Enter(const shared_ptr<CGuard>& Entity);
	virtual void ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CGuard>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CGuard>& Entity);
};

//=========================================================================================================================

class CGuardReturnState : public CState<CGuard>
{
private:
	CGuardReturnState() = default;
	virtual ~CGuardReturnState() = default;

public:
	static CGuardReturnState* GetInstance();

	virtual void Enter(const shared_ptr<CGuard>& Entity);
	virtual void ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CGuard>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CGuard>& Entity);
};

//=========================================================================================================================

class CGuardAssembleState : public CState<CGuard>
{
private:
	CGuardAssembleState() = default;
	virtual ~CGuardAssembleState() = default;

public:
	static CGuardAssembleState* GetInstance();

	virtual void Enter(const shared_ptr<CGuard>& Entity);
	virtual void ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CGuard>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CGuard>& Entity);
};

//=========================================================================================================================

class CGuardShootingState : public CState<CGuard>
{
private:
	CGuardShootingState() = default;
	virtual ~CGuardShootingState() = default;

public:
	static CGuardShootingState* GetInstance();

	virtual void Enter(const shared_ptr<CGuard>& Entity);
	virtual void ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CGuard>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CGuard>& Entity);
};

//=========================================================================================================================

class CGuardHitState : public CState<CGuard>
{
private:
	CGuardHitState() = default;
	virtual ~CGuardHitState() = default;

public:
	static CGuardHitState* GetInstance();

	virtual void Enter(const shared_ptr<CGuard>& Entity);
	virtual void ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CGuard>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CGuard>& Entity);
};

//=========================================================================================================================

class CGuardDyingState : public CState<CGuard>
{
private:
	CGuardDyingState() = default;
	virtual ~CGuardDyingState() = default;

public:
	static CGuardDyingState* GetInstance();

	virtual void Enter(const shared_ptr<CGuard>& Entity);
	virtual void ProcessInput(const shared_ptr<CGuard>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CGuard>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CGuard>& Entity);
};
