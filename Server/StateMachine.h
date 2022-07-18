#pragma once
#include "State.h"

template<typename EntityType>
class CStateMachine
{
private:
	shared_ptr<EntityType> m_Owner{};
							
	CState<EntityType>*	   m_CurrentState{};
	CState<EntityType>*	   m_PreviousState{};

public:
	CStateMachine(const shared_ptr<EntityType>& Owner) :
		m_Owner{ Owner }
	{

	}

	~CStateMachine() = default;

	void SetOwner(const shared_ptr<EntityType>& Owner)
	{
		m_Owner = Owner;
	}

	void SetCurrentState(CState<EntityType>* State)
	{
		m_CurrentState = State;

		if (m_CurrentState)
		{
			m_CurrentState->Enter(m_Owner);
		}
	}

	CState<EntityType>* GetCurrentState() const
	{
		return m_CurrentState;
	}

	CState<EntityType>* GetPreviousState() const
	{
		return m_PreviousState;
	}

	void ChangeState(CState<EntityType>* NewState)
	{
		if (NewState && (m_CurrentState != NewState))
		{
			m_CurrentState->Exit(m_Owner);
			m_PreviousState = m_CurrentState;
			m_CurrentState = NewState;
			m_CurrentState->Enter(m_Owner);
		}
	}

	bool IsInState(CState<EntityType>* State) const
	{
		if (m_CurrentState)
		{
			return typeid(*m_CurrentState) == typeid(*State);
		}

		return false;
	}

	void ProcessInput(float ElapsedTime, UINT InputMask)
	{
		if (m_CurrentState)
		{
			m_CurrentState->ProcessInput(m_Owner, ElapsedTime, InputMask);
		}
	}

	void Update(float ElapsedTime)
	{
		if (m_CurrentState)
		{
			m_CurrentState->Update(m_Owner, ElapsedTime);
		}
	}
};
