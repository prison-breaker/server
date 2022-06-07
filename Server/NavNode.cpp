#include "stdafx.h"
#include "NavNode.h"

void CNavNode::SetTriangle(const TRIANGLE& Triangle)
{
	m_Triangle = Triangle;

	CalculateCenterSides();
	CalculateCentroid();
}

const TRIANGLE& CNavNode::GetTriangle() const
{
	return m_Triangle;
}

void CNavNode::SetVisit(bool IsVisited)
{
	m_IsVisited = IsVisited;
}

bool CNavNode::IsVisited() const
{
	return m_IsVisited;
}

void CNavNode::CalculateCenterSides()
{
	m_Triangle.m_CenterSides[0] = { 0.5f * (m_Triangle.m_Vertices[0].x + m_Triangle.m_Vertices[1].x), 0.5f * (m_Triangle.m_Vertices[0].y + m_Triangle.m_Vertices[1].y), 0.5f * (m_Triangle.m_Vertices[0].z + m_Triangle.m_Vertices[1].z) };
	m_Triangle.m_CenterSides[1] = { 0.5f * (m_Triangle.m_Vertices[1].x + m_Triangle.m_Vertices[2].x), 0.5f * (m_Triangle.m_Vertices[1].y + m_Triangle.m_Vertices[2].y), 0.5f * (m_Triangle.m_Vertices[1].z + m_Triangle.m_Vertices[2].z) };
	m_Triangle.m_CenterSides[2] = { 0.5f * (m_Triangle.m_Vertices[2].x + m_Triangle.m_Vertices[0].x), 0.5f * (m_Triangle.m_Vertices[2].y + m_Triangle.m_Vertices[0].y), 0.5f * (m_Triangle.m_Vertices[2].z + m_Triangle.m_Vertices[0].z) };
}

void CNavNode::CalculateCentroid()
{
	m_Triangle.m_Centroid = {
		0.33f * (m_Triangle.m_Vertices[0].x + m_Triangle.m_Vertices[1].x + m_Triangle.m_Vertices[2].x),
		0.33f * (m_Triangle.m_Vertices[0].y + m_Triangle.m_Vertices[1].y + m_Triangle.m_Vertices[2].y),
		0.33f * (m_Triangle.m_Vertices[0].z + m_Triangle.m_Vertices[1].z + m_Triangle.m_Vertices[2].z)
	};
}

void CNavNode::CalculateH(const shared_ptr<CNavNode>& TargetNavNode)
{
	m_H = Math::Distance(m_Triangle.m_Centroid, TargetNavNode->m_Triangle.m_Centroid);
}

float CNavNode::GetH() const
{
	return m_H;
}

void CNavNode::CalculateG(const shared_ptr<CNavNode>& ParentNavNode)
{
	float Distance{ Math::Distance(m_Triangle.m_Centroid, ParentNavNode->m_Triangle.m_Centroid) };

	m_G = ParentNavNode->GetG() + Distance;
}

float CNavNode::GetG() const
{
	return m_G;
}

void CNavNode::CalculateF(const shared_ptr<CNavNode>& ParentNavNode, const shared_ptr<CNavNode>& TargetNavNode)
{
	CalculateH(TargetNavNode);
	CalculateG(ParentNavNode);

	m_F = m_H + m_G;
}

float CNavNode::GetF() const
{
	return m_F;
}

UINT CNavNode::CalculateNeighborSideIndex(const shared_ptr<CNavNode>& NavNode)
{
	UINT TotalNeighborSideIndex{};

	for (UINT i = 0; i < 3; ++i)
	{
		for (UINT j = 0; j < 3; ++j)
		{			
			if (Vector3::IsEqual(m_Triangle.m_Vertices[i], NavNode->m_Triangle.m_Vertices[j]))
			{
				TotalNeighborSideIndex += i;
				continue;
			}
		}
	}

	switch (TotalNeighborSideIndex)
	{
	case 1:
		return 0;
	case 2:
		return 2;
	case 3:
		return 1;
	}

	return 4;
}

vector<shared_ptr<CNavNode>>& CNavNode::GetNeighborNavNodes()
{
	return m_NeighborNavNodes;
}

void CNavNode::SetParent(const shared_ptr<CNavNode>& ParentNavNode)
{
	m_Parent = ParentNavNode;
}

const shared_ptr<CNavNode>& CNavNode::GetParent() const
{
	return m_Parent;
}

void CNavNode::Reset()
{
	m_IsVisited = false;
	m_H = m_G = m_F = 0;
	m_Parent = nullptr;
}
