#pragma once
#include "State.h"

class CPlayer;

class CPlayerIdleState : public CState<CPlayer>
{
private:
	CPlayerIdleState() = default;
	virtual ~CPlayerIdleState() = default;

public:
	static CPlayerIdleState* GetInstance();

	virtual void Enter(const shared_ptr<CPlayer>& Entity);
	virtual void ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CPlayer>& Entity);
};

//=========================================================================================================================

class CPlayerWalkingState : public CState<CPlayer>
{
private:
	CPlayerWalkingState() = default;
	virtual ~CPlayerWalkingState() = default;

public:
	static CPlayerWalkingState* GetInstance();

	virtual void Enter(const shared_ptr<CPlayer>& Entity);
	virtual void ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CPlayer>& Entity);
};

//=========================================================================================================================

class CPlayerRunningState : public CState<CPlayer>
{
private:
	CPlayerRunningState() = default;
	virtual ~CPlayerRunningState() = default;

public:
	static CPlayerRunningState* GetInstance();

	virtual void Enter(const shared_ptr<CPlayer>& Entity);
	virtual void ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CPlayer>& Entity);
};

//=========================================================================================================================

class CPlayerPunchingState : public CState<CPlayer>
{
private:
	CPlayerPunchingState() = default;
	virtual ~CPlayerPunchingState() = default;

public:
	static CPlayerPunchingState* GetInstance();

	virtual void Enter(const shared_ptr<CPlayer>& Entity);
	virtual void ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CPlayer>& Entity);
};

//=========================================================================================================================

class CPlayerShootingState : public CState<CPlayer>
{
private:
	CPlayerShootingState() = default;
	virtual ~CPlayerShootingState() = default;

public:
	static CPlayerShootingState* GetInstance();

	virtual void Enter(const shared_ptr<CPlayer>& Entity);
	virtual void ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CPlayer>& Entity);
};

//=========================================================================================================================

class CPlayerDyingState : public CState<CPlayer>
{
private:
	CPlayerDyingState() = default;
	virtual ~CPlayerDyingState() = default;

public:
	static CPlayerDyingState* GetInstance();

	virtual void Enter(const shared_ptr<CPlayer>& Entity);
	virtual void ProcessInput(const shared_ptr<CPlayer>& Entity, float ElapsedTime, UINT InputMask);
	virtual void Update(const shared_ptr<CPlayer>& Entity, float ElapsedTime);
	virtual void Exit(const shared_ptr<CPlayer>& Entity);
};
