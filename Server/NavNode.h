#pragma once

struct TRIANGLE
{
	XMFLOAT3 m_Vertices[3]{};
	XMFLOAT3 m_CenterSides[3]{};
	XMFLOAT3 m_Centroid{};
};

class CNavNode
{
	friend class CNavMesh;

private:
	bool				         m_IsVisited{};

	float				         m_F{};
	float                        m_H{};
	float                        m_G{};

	TRIANGLE			         m_Triangle{};

	vector<shared_ptr<CNavNode>> m_NeighborNavNodes{};
	shared_ptr<CNavNode>         m_Parent{};

public:
	void SetTriangle(const TRIANGLE& Triangle);
	const TRIANGLE& GetTriangle() const;

	void SetVisit(bool IsVisited);
	bool IsVisited() const;

	void CalculateCenterSides();
	void CalculateCentroid();

	void CalculateH(const shared_ptr<CNavNode>& TargetNavNode);
	float GetH() const;

	void CalculateG(const shared_ptr<CNavNode>& ParentNavNode);
	float GetG() const;

	void CalculateF(const shared_ptr<CNavNode>& ParentNavNode, const shared_ptr<CNavNode>& TargetNavNode);
	float GetF() const;

	UINT CalculateNeighborSideIndex(const shared_ptr<CNavNode>& NavNode);

	vector<shared_ptr<CNavNode>>& GetNeighborNavNodes();

	void SetParent(const shared_ptr<CNavNode>& ParentNavNode);
	const shared_ptr<CNavNode>& GetParent() const;

	void Reset();
};

struct compare
{
	bool operator ()(const shared_ptr<CNavNode>& NavNode1, const shared_ptr<CNavNode>& NavNode2)
	{
		return NavNode1->GetF() > NavNode2->GetF();
	}
};
