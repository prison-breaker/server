#pragma once
#include "EventTrigger.h"

class COpenDoorEventTrigger : public CEventTrigger
{
private:
	float m_DoorAngle{};

public:
	COpenDoorEventTrigger();
	virtual ~COpenDoorEventTrigger() = default;

	virtual void Reset();

	virtual bool CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition);

	virtual void Update(float ElapsedTime);
};

//=========================================================================================================================

class CPowerDownEventTrigger : public CEventTrigger
{
private:
	bool  m_IsOpened{};

	float m_PanelAngle{};

public:
	CPowerDownEventTrigger();
	virtual ~CPowerDownEventTrigger() = default;

	virtual void Reset();

	virtual void InteractEventTrigger(UINT CallerIndex);
	virtual void Update(float ElapsedTime);
};

//=========================================================================================================================

class CSirenEventTrigger : public CEventTrigger
{
public:
	CSirenEventTrigger();
	virtual ~CSirenEventTrigger() = default;

	virtual void InteractEventTrigger(UINT CallerIndex);
};

//=========================================================================================================================

class COpenGateEventTrigger : public CEventTrigger
{
private:
	bool  m_UsedKeyIndices[MAX_PLAYER_CAPACITY]{};

	float m_GateAngle{};

public:
	COpenGateEventTrigger();
	virtual ~COpenGateEventTrigger() = default;

	virtual void Reset();

	virtual bool CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition);

	virtual void InteractEventTrigger(UINT CallerIndex);
	virtual void Update(float ElapsedTime);
};

//=========================================================================================================================

class CGetPistolEventTrigger : public CEventTrigger
{
public:
	CGetPistolEventTrigger();
	virtual ~CGetPistolEventTrigger() = default;

	virtual void Reset();

	virtual void InteractEventTrigger(UINT CallerIndex);
};

//=========================================================================================================================

class CGetKeyEventTrigger : public CEventTrigger
{
public:
	CGetKeyEventTrigger();
	virtual ~CGetKeyEventTrigger() = default;

	virtual void Reset();

	virtual void InteractEventTrigger(UINT CallerIndex);
};
