#pragma once
#include "GameObject.h"
#include "State_Player.h"

class CPlayer : public CGameObject
{
private:
	XMFLOAT3						   m_Rotation{};

	UINT							   m_Health{ 100 };

	float							   m_Speed{};
	XMFLOAT3						   m_MovingDirection{};

	shared_ptr<CGameObject>            m_PistolFrame{};
									   
	shared_ptr<CStateMachine<CPlayer>> m_StateMachine{};

public:
	CPlayer() = default;
	virtual ~CPlayer() = default;

	virtual void Initialize();

	virtual void Animate(float ElapsedTime);

	void SetHealth(UINT Health);
	UINT GetHealth() const;

	void SetSpeed(float Speed);
	float GetSpeed() const;

	void SetMovingDirection(const XMFLOAT3& MovingDirection);
	const XMFLOAT3& GetMovingDirection() const;

	shared_ptr<CStateMachine<CPlayer>> GetStateMachine() const;

	void AcquirePistol();
	bool HasPistol() const;
	bool IsEquippedPistol() const;

	bool SwapWeapon(WEAPON_TYPE WeaponType);

	void ApplySlidingVectorToPosition(const shared_ptr<CNavMesh>& NavMesh, XMFLOAT3& NewPosition);

	void Rotate(float Pitch, float Yaw, float Roll, float ElapsedTime, float NearestHitDistance);

	bool IsCollidedByGuard(const XMFLOAT3& NewPosition);
	bool IsCollidedByEventTrigger(const XMFLOAT3& NewPosition, bool IsInteracted);

	void ProcessInput(float ElapsedTime, UINT InputMask);
};
