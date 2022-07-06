#pragma once

class CGameObject;

class CEventTrigger : public enable_shared_from_this<CEventTrigger>
{
protected:
	MSG_TYPE						m_Type{};

	bool							m_IsActive{};
	bool							m_IsInteracted{};

	XMFLOAT3						m_ToTrigger{};
	float							m_ActiveFOV{};

	XMFLOAT3						m_TriggerArea[4]{}; // Vertices

	vector<shared_ptr<CGameObject>> m_EventObjects{};

public:
	CEventTrigger(MSG_TYPE Type);
	virtual ~CEventTrigger() = default;

	virtual bool CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition);

	virtual void InteractEventTrigger();
	virtual void Update(float ElapsedTime);

	void LoadEventTriggerFromFile(tifstream& InFile);

	MSG_TYPE GetType() const;

	void SetActive(bool IsActive);
	bool IsActive() const;

	void SetInteracted(bool IsInteracted);
	bool IsInteracted() const;

	void CalculateTriggerAreaByGuard(const XMFLOAT3& Position);

	void InsertEventObject(const shared_ptr<CGameObject>& EventObject);
	shared_ptr<CGameObject> GetEventObject(UINT Index);

	bool IsInTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& LookDirection);

	void DeleteThisEventTrigger();
};
