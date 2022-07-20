#pragma once
#include "GameObject.h"
#include "State_Player.h"

class CPlayer : public CGameObject
{
private:
	UINT							   m_ID{};

	XMFLOAT3						   m_Rotation{};

	UINT							   m_Health{ 100 };

	float							   m_Speed{};
	XMFLOAT3						   m_MovingDirection{};

	shared_ptr<CGameObject>            m_PistolFrame{};
		
	bool							   m_HasKey{};

	XMFLOAT3						   m_CameraPosition{};

	shared_ptr<CStateMachine<CPlayer>> m_StateMachine{};

public:
	CPlayer() = default;
	virtual ~CPlayer() = default;

	virtual void Initialize();
	virtual void Reset(const XMFLOAT4X4& TransformMatrix);

	virtual void Animate(float ElapsedTime);

	void SetID(UINT ID);
	UINT GetID() const;

	void SetHealth(UINT Health);
	UINT GetHealth() const;

	void SetSpeed(float Speed);
	float GetSpeed() const;

	void SetMovingDirection(const XMFLOAT3& MovingDirection);
	const XMFLOAT3& GetMovingDirection() const;

	void SetCameraPosition(const XMFLOAT3& CameraPosition);
	const XMFLOAT3& GetCameraPosition() const;

	shared_ptr<CStateMachine<CPlayer>> GetStateMachine() const;

	void ManagePistol(bool HasPistol);
	bool HasPistol() const;
	bool IsEquippedPistol() const;

	void ManageKey(bool HasKey);
	bool HasKey() const;

	bool SwapWeapon(WEAPON_TYPE WeaponType);

	void ApplySlidingVectorToPosition(const shared_ptr<CNavMesh>& NavMesh, XMFLOAT3& NewPosition);

	bool IsCollidedByPlayer(const XMFLOAT3& NewPosition);
	bool IsCollidedByGuard(const XMFLOAT3& NewPosition);
	bool IsCollidedByEventTrigger(const XMFLOAT3& NewPosition, bool IsInteracted);

	void ProcessInput(float ElapsedTime, UINT InputMask);
};
