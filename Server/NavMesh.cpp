#include "stdafx.h"
#include "NavMesh.h"
#include "NavNode.h"

void CNavMesh::LoadNavMeshFromFile(const tstring& FileName)
{
	tstring Token{};
	vector<XMFLOAT3> Vertices{};

	tcout << FileName << TEXT(" 로드 시작...") << endl;

	tifstream InFile{ FileName, ios::binary };

	while (true)
	{
		File::ReadStringFromFile(InFile, Token);

		if (Token == TEXT("<Positions>"))
		{
			UINT VertexCount{ File::ReadIntegerFromFile(InFile) };

			if (VertexCount > 0)
			{
				Vertices.resize(VertexCount);

				InFile.read(reinterpret_cast<TCHAR*>(Vertices.data()), VertexCount * sizeof(XMFLOAT3));
			}
		}
		else if (Token == TEXT("<Indices>"))
		{
			vector<UINT> Indices{};
			UINT IndexCount{ File::ReadIntegerFromFile(InFile) };

			if (IndexCount > 0)
			{
				Indices.resize(IndexCount);

				InFile.read(reinterpret_cast<TCHAR*>(Indices.data()), IndexCount * sizeof(UINT));

				for (UINT i = 0; i < IndexCount; i += 3)
				{
					shared_ptr<CNavNode> NewNavNode{ make_shared<CNavNode>() };
					TRIANGLE Triangle{};

					Triangle.m_Vertices[0] = Vertices[Indices[i] - 1];
					Triangle.m_Vertices[1] = Vertices[Indices[i + 1] - 1];
					Triangle.m_Vertices[2] = Vertices[Indices[i + 2] - 1];

					NewNavNode->SetTriangle(Triangle);
					InsertNode(NewNavNode);
				}
			}
		}
		else if (Token == TEXT("</NavMesh>"))
		{
			break;
		}
	}

	tcout << FileName << TEXT(" 로드 완료...(정점 수: ") << m_NavNodes.size() << ")" << endl << endl;
}

vector<shared_ptr<CNavNode>>& CNavMesh::GetNavNodes()
{
	return m_NavNodes;
}

void CNavMesh::InsertNode(const shared_ptr<CNavNode>& NewNavNode)
{
	if (m_NavNodes.empty())
	{
		m_NavNodes.push_back(NewNavNode);
	}
	else
	{
		// 기존의 노드와 이웃한 꼭짓점이 있다면 추가한다.
		for (const auto& NavNode : m_NavNodes)
		{
			UINT NeighborVertexCount{};

			for (UINT i = 0; i < 3; ++i)
			{
				XMFLOAT3 Vertex1{ NavNode->m_Triangle.m_Vertices[i] };

				for (UINT j = 0; j < 3; ++j)
				{
					XMFLOAT3 Vertex2{ NewNavNode->m_Triangle.m_Vertices[j] };

					if (Vector3::IsEqual(Vertex1, Vertex2))
					{
						NeighborVertexCount += 1;
						continue;
					}
				}
			}

			if (NeighborVertexCount == 2)
			{
				NavNode->m_NeighborNavNodes.push_back(NewNavNode);
				NewNavNode->m_NeighborNavNodes.push_back(NavNode);
			}
		}

		m_NavNodes.push_back(NewNavNode);
	}
}

UINT CNavMesh::GetNodeIndex(const XMFLOAT3& Position)
{
	UINT NavNodeSize{ static_cast<UINT>(m_NavNodes.size()) };

	for (UINT i = 0; i < NavNodeSize; ++i)
	{
		if (Math::IsInTriangle(m_NavNodes[i]->m_Triangle.m_Vertices[0], m_NavNodes[i]->m_Triangle.m_Vertices[1], m_NavNodes[i]->m_Triangle.m_Vertices[2], Position))
		{
			return i;
		}
	}

	// 포지션이 NavMesh위에 없을 경우 제일 인접한 Index를 반환한다.
	float NearestDistance{ FLT_MAX };
	float Distance{};
	UINT Index{};

	for (UINT i = 0; i < NavNodeSize; ++i)
	{
		Distance = Math::Distance(m_NavNodes[i]->GetTriangle().m_Centroid, Position);

		if (Distance < NearestDistance)
		{
			NearestDistance = Distance;
			Index = i;
		}
	}

	return Index;
}
