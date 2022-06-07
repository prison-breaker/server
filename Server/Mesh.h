#pragma once

class CGameObject;

class CMesh
{
protected:
	tstring				 m_Name{};
			        	 
	vector<XMFLOAT3>	 m_Positions{};
	vector<vector<UINT>> m_Indices{};
						 	 
	BoundingBox			 m_BoundingBox{};

public:
	CMesh() = default;
	virtual ~CMesh() = default;

	void LoadMeshInfoFromFile(tifstream& InFile);

	void SetName(const tstring& Name);
	const tstring& GetName() const;

	void SetBoundingBox(const BoundingBox& BoundingBox);
	const BoundingBox& GetBoundingBox() const;

	bool CheckRayIntersection(const XMFLOAT3& RayOrigin, const XMFLOAT3& RayDirection, const XMMATRIX& WorldMatrix, float& HitDistance);
};

//=========================================================================================================================

class CSkinnedMesh : public CMesh
{
private:
	UINT							m_BoneCount{};

	vector<XMUINT4>					m_BoneIndices{};
	vector<XMFLOAT4>				m_BoneWeights{};

	vector<XMFLOAT4X4>				m_BoneOffsetMatrixes{};
	XMFLOAT4X4*						m_MappedBoneOffsetMatrixes{};

	vector<shared_ptr<CGameObject>> m_BoneFrameCaches{};
	XMFLOAT4X4*						m_MappedBoneTransformMatrixes{};

public:
	CSkinnedMesh() = default;
	CSkinnedMesh(const CSkinnedMesh& Rhs);
	virtual ~CSkinnedMesh() = default;

	virtual void UpdateShaderVariables();

	void LoadSkinInfoFromFile(tifstream& InFile);
	
	void SetBoneFrameCaches(const vector <shared_ptr<CGameObject>>& BoneFrames);
	void SetBoneTransformInfo(XMFLOAT4X4* MappedBoneTransformMatrixes);
};
