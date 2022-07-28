#include "stdafx.h"
#include "Mesh.h"
#include "GameObject.h"

void CMesh::LoadMeshInfoFromFile(tifstream& InFile)
{
	tstring Token{};

	File::ReadStringFromFile(InFile, m_Name);

	while (true)
	{
		File::ReadStringFromFile(InFile, Token);

		if (Token == TEXT("<Positions>"))
		{
			UINT VertexCount{ File::ReadIntegerFromFile(InFile) };

			if (VertexCount > 0)
			{
				m_Positions.resize(VertexCount);

				InFile.read(reinterpret_cast<TCHAR*>(m_Positions.data()), sizeof(XMFLOAT3) * VertexCount);
			}
		}
		else if (Token == TEXT("<SubMeshes>"))
		{
			UINT SubMeshCount{ File::ReadIntegerFromFile(InFile) };

			if (SubMeshCount > 0)
			{
				m_Indices.resize(SubMeshCount);

				for (UINT i = 0; i < SubMeshCount; ++i)
				{
					// <Indices>
					File::ReadStringFromFile(InFile, Token);

					UINT IndexCount{ File::ReadIntegerFromFile(InFile) };

					if (IndexCount > 0)
					{
						m_Indices[i].resize(IndexCount);

						InFile.read(reinterpret_cast<TCHAR*>(m_Indices[i].data()), sizeof(UINT) * m_Indices[i].size());
					}
				}
			}
		}
		else if (Token == TEXT("<Bounds>"))
		{
			InFile.read(reinterpret_cast<TCHAR*>(&m_BoundingBox.Center), sizeof(XMFLOAT3));
			InFile.read(reinterpret_cast<TCHAR*>(&m_BoundingBox.Extents), sizeof(XMFLOAT3));
		}
		else if (Token == TEXT("</Mesh>") || Token == TEXT("</SkinnedMesh>"))
		{
			break;
		}
	}
}

void CMesh::SetName(const tstring& Name)
{
	m_Name = Name;
}

const tstring& CMesh::GetName() const
{
	return m_Name;
}

void CMesh::SetBoundingBox(const BoundingBox& BoundingBox)
{
	m_BoundingBox = BoundingBox;
}

const BoundingBox& CMesh::GetBoundingBox() const
{
	return m_BoundingBox;
}

bool CMesh::CheckRayIntersection(const XMFLOAT3& RayOrigin, const XMFLOAT3& RayDirection, const XMMATRIX& WorldMatrix, float& HitDistance)
{
	bool Intersected{};

	UINT SubMeshCount{ static_cast<UINT>(m_Indices.size()) };
	float NearestHitDistance{ FLT_MAX };

	for (UINT i = 0; i < SubMeshCount; ++i)
	{
		UINT PrimitiveCount{ static_cast<UINT>(m_Indices[i].size()) };

		for (UINT j = 0; j < PrimitiveCount; j += 3)
		{
			XMVECTOR Vertex1{ XMVector3TransformCoord(XMLoadFloat3(&m_Positions[m_Indices[i][j]]), WorldMatrix) };
			XMVECTOR Vertex2{ XMVector3TransformCoord(XMLoadFloat3(&m_Positions[m_Indices[i][j + 1]]), WorldMatrix) };
			XMVECTOR Vertex3{ XMVector3TransformCoord(XMLoadFloat3(&m_Positions[m_Indices[i][j + 2]]), WorldMatrix) };

			// 메쉬의 모든 프리미티브(삼각형)들에 대하여 픽킹 광선과의 충돌을 검사한다.
			if (TriangleTests::Intersects(XMLoadFloat3(&RayOrigin), XMLoadFloat3(&RayDirection), Vertex1, Vertex2, Vertex3, HitDistance))
			{
				if (HitDistance < NearestHitDistance)
				{
					NearestHitDistance = HitDistance;
					Intersected = true;
				}
			}
		}
	}

	HitDistance = NearestHitDistance;

	return Intersected;
}

//=========================================================================================================================

CSkinnedMesh::CSkinnedMesh(const CSkinnedMesh& Rhs)
{
	m_Name = Rhs.m_Name;

	m_Positions = Rhs.m_Positions;
	m_Indices = Rhs.m_Indices;

	m_BoundingBox = Rhs.m_BoundingBox;
}

void CSkinnedMesh::UpdateShaderVariables()
{

}

void CSkinnedMesh::LoadSkinInfoFromFile(tifstream& InFile)
{
	tstring Token{};

	while (true)
	{
		File::ReadStringFromFile(InFile, Token);

		if (Token == TEXT("<BoneOffsetMatrixes>"))
		{
			m_BoneCount = File::ReadIntegerFromFile(InFile);

			if (m_BoneCount > 0)
			{
				m_BoneOffsetMatrixes.reserve(m_BoneCount);

				for (UINT i = 0; i < m_BoneCount; ++i)
				{
					XMFLOAT4X4 BoneOffsetMatrix{};

					InFile.read(reinterpret_cast<TCHAR*>(&BoneOffsetMatrix), sizeof(XMFLOAT4X4));

					m_BoneOffsetMatrixes.push_back(BoneOffsetMatrix);
				}
			}
		}
		else if (Token == TEXT("<BoneIndices>"))
		{
			UINT VertexCount{ File::ReadIntegerFromFile(InFile) };

			if (VertexCount > 0)
			{
				m_BoneIndices.resize(VertexCount);

				InFile.read(reinterpret_cast<TCHAR*>(m_BoneIndices.data()), sizeof(XMUINT4) * VertexCount);
			}
		}
		else if (Token == TEXT("<BoneWeights>"))
		{
			UINT VertexCount{ File::ReadIntegerFromFile(InFile) };

			if (VertexCount > 0)
			{
				m_BoneWeights.resize(VertexCount);
				
				InFile.read(reinterpret_cast<TCHAR*>(m_BoneWeights.data()), sizeof(XMFLOAT4) * VertexCount);
			}
		}
		else if (Token == TEXT("</SkinInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::SetBoneFrameCaches(const vector<shared_ptr<CGameObject>>& BoneFrames)
{
	m_BoneFrameCaches.assign(BoneFrames.begin(), BoneFrames.end());
}

void CSkinnedMesh::SetBoneTransformInfo(XMFLOAT4X4* MappedBoneTransformMatrixes)
{
	if (MappedBoneTransformMatrixes)
	{
		m_MappedBoneTransformMatrixes = MappedBoneTransformMatrixes;
	}
}
