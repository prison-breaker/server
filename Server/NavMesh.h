#pragma once
#include "NavNode.h"

class CNavMesh
{
private:
	vector<shared_ptr<CNavNode>> m_NavNodes{};

public:
	CNavMesh() = default;
	~CNavMesh() = default;

	void LoadNavMeshFromFile(const tstring& FileName);

	vector<shared_ptr<CNavNode>>& GetNavNodes();

	void InsertNode(const shared_ptr<CNavNode>& NewNavNode);

	UINT GetNodeIndex(const XMFLOAT3& Position);
};
