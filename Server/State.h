#pragma once

template<typename EntityType>
class CState
{
public:
	CState() = default;
	virtual ~CState() = default;

	virtual void Enter(const shared_ptr<EntityType>& Entity) = 0;
	virtual void ProcessInput(const shared_ptr<EntityType>& Entity, float ElapsedTime, UINT InputMask) = 0;
	virtual void Update(const shared_ptr<EntityType>& Entity, float ElapsedTime) = 0;
	virtual void Exit(const shared_ptr<EntityType>& Entity) = 0;
};
