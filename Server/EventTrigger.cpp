#include "stdafx.h"
#include "Server.h"

void CEventTrigger::Reset()
{
	m_IsActive = true;
	m_IsInteracted = false;
}

bool CEventTrigger::CanPassTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& NewPosition)
{
	return true;
}

void CEventTrigger::InteractEventTrigger(UINT CallerIndex)
{
	m_IsInteracted = true;
}

void CEventTrigger::Update(float ElapsedTime)
{

}

void CEventTrigger::LoadEventTriggerFromFile(tifstream& InFile)
{
	tstring Token{};
	UINT TargetRootIndex{};

	while (true)
	{
		File::ReadStringFromFile(InFile, Token);

		if (Token == TEXT("<TriggerAreas>"))
		{
			InFile.read(reinterpret_cast<TCHAR*>(m_TriggerArea), 4 * sizeof(XMFLOAT3));
		}
		else if (Token == TEXT("<ToTrigger>"))
		{
			InFile.read(reinterpret_cast<TCHAR*>(&m_ToTrigger), sizeof(XMFLOAT3));
		}
		else if (Token == TEXT("<TargetRootIndex>"))
		{
			TargetRootIndex = File::ReadIntegerFromFile(InFile);
		}
		else if (Token == TEXT("<TargetObject>"))
		{
			UINT TargetObjectCount{ File::ReadIntegerFromFile(InFile) };
			vector<vector<shared_ptr<CGameObject>>>& GameObjects{ CServer::m_GameObjects };

			m_EventObjects.reserve(TargetObjectCount);

			for (UINT i = 0; i < TargetObjectCount; ++i)
			{
				File::ReadStringFromFile(InFile, Token);

				shared_ptr<CGameObject> TargetObject{ GameObjects[OBJECT_TYPE_STRUCTURE][TargetRootIndex]->FindFrame(Token) };

				if (TargetObject)
				{
					m_EventObjects.push_back(TargetObject);
				}
			}
			break;
		}
	}
}

void CEventTrigger::SetActive(bool IsActive)
{
	m_IsActive = IsActive;
}

bool CEventTrigger::IsActive() const
{
	return m_IsActive;
}

void CEventTrigger::SetInteracted(bool IsInteracted)
{
	m_IsInteracted = IsInteracted;
}

bool CEventTrigger::IsInteracted() const
{
	return m_IsInteracted;
}

void CEventTrigger::CalculateTriggerAreaByGuard(const XMFLOAT3& Position)
{
	m_TriggerArea[0].x = Position.x - 3.0f;
	m_TriggerArea[0].z = Position.z - 3.0f;

	m_TriggerArea[1].x = Position.x - 3.0f;
	m_TriggerArea[1].z = Position.z + 3.0f;

	m_TriggerArea[2].x = Position.x + 3.0f;
	m_TriggerArea[2].z = Position.z + 3.0f;

	m_TriggerArea[3].x = Position.x + 3.0f;
	m_TriggerArea[3].z = Position.z - 3.0f;
}

void CEventTrigger::InsertEventObject(const shared_ptr<CGameObject>& EventObject)
{
	if (EventObject)
	{
		m_EventObjects.push_back(EventObject);
	}
}

shared_ptr<CGameObject> CEventTrigger::GetEventObject(UINT Index)
{
	if (Index < 0 || Index >= m_EventObjects.size())
	{
		return nullptr;
	}

	return m_EventObjects[Index];
}

bool CEventTrigger::IsInTriggerArea(const XMFLOAT3& Position, const XMFLOAT3& LookDirection)
{
	if (m_IsActive && !m_IsInteracted)
	{
		for (UINT i = 0; i < 2; ++i)
		{
			if (Math::IsInTriangle(m_TriggerArea[0], m_TriggerArea[i + 1], m_TriggerArea[i + 2], Position))
			{
				return true;
			}
		}
	}

	return false;
}

bool CEventTrigger::IsInActiveAngle(const XMFLOAT3& LookDirection)
{
	if (Vector3::Angle(LookDirection, m_ToTrigger) <= m_ActiveFOV)
	{
		return true;
	}

	return false;
}
