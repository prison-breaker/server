#include "stdafx.h"
#include "GameObject.h"

shared_ptr<LOADED_MODEL_INFO> CGameObject::LoadObjectFromFile(const tstring& FileName, unordered_map<tstring, shared_ptr<CMesh>>& MeshCaches)
{
	tstring Token{};

	shared_ptr<LOADED_MODEL_INFO> ModelInfo{ make_shared<LOADED_MODEL_INFO>() };

	tifstream InFile{ FileName, ios::binary };

	while (true)
	{
		File::ReadStringFromFile(InFile, Token);

		if (Token == TEXT("<Hierarchy>"))
		{
			tcout << FileName << TEXT(" 모델 로드 시작...") << endl;
			ModelInfo->m_Model = CGameObject::LoadModelInfoFromFile(InFile, MeshCaches);
		}
		else if (Token == TEXT("</Hierarchy>"))
		{
			tcout << FileName << TEXT(" 모델 로드 완료...") << endl;
			break;
		}
	}

	File::ReadStringFromFile(InFile, Token);

	if (Token == TEXT("<Animation>"))
	{
		tcout << FileName << TEXT(" 애니메이션 로드 시작...") << endl;
		CGameObject::LoadAnimationInfoFromFile(InFile, ModelInfo);
		tcout << FileName << TEXT(" 애니메이션 로드 완료...") << endl;
	}

	return ModelInfo;
}

shared_ptr<CGameObject> CGameObject::LoadModelInfoFromFile(tifstream& InFile, unordered_map<tstring, shared_ptr<CMesh>>& MeshCaches)
{
	tstring Token{};
	shared_ptr<CGameObject> NewObject{};

	while (true)
	{
		File::ReadStringFromFile(InFile, Token);

		if (Token == TEXT("<Frame>"))
		{
			NewObject = make_shared<CGameObject>();
			NewObject->SetActive(true);

			File::ReadStringFromFile(InFile, NewObject->m_FrameName);
		}
		else if (Token == TEXT("<TransformMatrix>"))
		{
			InFile.read(reinterpret_cast<TCHAR*>(&NewObject->m_TransformMatrix), sizeof(XMFLOAT4X4));
		}
		else if (Token == TEXT("<Mesh>"))
		{
			File::ReadStringFromFile(InFile, Token);

			NewObject->SetMesh(MeshCaches[Token]);
			NewObject->SetBoundingBox(make_shared<BoundingBox>());
		}
		else if (Token == TEXT("<SkinnedMesh>"))
		{
			File::ReadStringFromFile(InFile, Token);

			shared_ptr<CSkinnedMesh> SkinnedMesh{ make_shared<CSkinnedMesh>(*(static_pointer_cast<CSkinnedMesh>(MeshCaches[Token]))) };

			SkinnedMesh->LoadSkinInfoFromFile(InFile);
			NewObject->SetMesh(SkinnedMesh);
			NewObject->SetBoundingBox(make_shared<BoundingBox>());
		}
		else if (Token == TEXT("<ChildCount>"))
		{
			UINT ChildCount{ File::ReadIntegerFromFile(InFile) };

			if (ChildCount > 0)
			{
				for (UINT i = 0; i < ChildCount; ++i)
				{
					shared_ptr<CGameObject> ChildObject{ CGameObject::LoadModelInfoFromFile(InFile, MeshCaches) };

					if (ChildObject)
					{
						NewObject->SetChild(ChildObject);
					}
				}
			}
		}
		else if (Token == TEXT("</Frame>"))
		{
			break;
		}
	}

	return NewObject;
}

void CGameObject::LoadAnimationInfoFromFile(tifstream& InFile, const shared_ptr<LOADED_MODEL_INFO>& ModelInfo)
{
	tstring Token{};

	unordered_map<tstring, shared_ptr<CGameObject>> BoneFrameCaches{};

	while (true)
	{
		File::ReadStringFromFile(InFile, Token);

		if (Token == TEXT("<FrameNames>"))
		{
			UINT SkinnedMeshFrameCount{ File::ReadIntegerFromFile(InFile) };

			ModelInfo->m_SkinnedMeshCaches.reserve(SkinnedMeshFrameCount);
			ModelInfo->m_BoneFrameCaches.resize(SkinnedMeshFrameCount);

			for (UINT i = 0; i < SkinnedMeshFrameCount; ++i)
			{
				File::ReadStringFromFile(InFile, Token);
				ModelInfo->m_SkinnedMeshCaches.push_back(ModelInfo->m_Model->FindSkinnedMesh(Token));

				UINT BoneCount{ File::ReadIntegerFromFile(InFile) };

				ModelInfo->m_BoneFrameCaches[i].reserve(BoneCount);

				for (UINT j = 0; j < BoneCount; ++j)
				{
					File::ReadStringFromFile(InFile, Token);

					if (BoneFrameCaches.count(Token))
					{
						ModelInfo->m_BoneFrameCaches[i].push_back(BoneFrameCaches[Token]);
					}
					else
					{
						shared_ptr<CGameObject> Frame{ ModelInfo->m_Model->FindFrame(Token) };

						ModelInfo->m_BoneFrameCaches[i].push_back(Frame);
						BoneFrameCaches.emplace(Token, Frame);
					}
				}

				ModelInfo->m_SkinnedMeshCaches.back()->SetBoneFrameCaches(ModelInfo->m_BoneFrameCaches[i]);
			}
		}
		else if (Token == TEXT("<AnimationClips>"))
		{
			UINT AnimationClipCount{ File::ReadIntegerFromFile(InFile) };

			ModelInfo->m_AnimationClips.reserve(AnimationClipCount);

			for (UINT i = 0; i < AnimationClipCount; ++i)
			{
				shared_ptr<CAnimationClip> AnimationClip{ make_shared<CAnimationClip>() };

				AnimationClip->LoadAnimationClipInfoFromFile(InFile, ModelInfo);
				ModelInfo->m_AnimationClips.push_back(AnimationClip);
			}
		}
		else if (Token == TEXT("</Animation>"))
		{
			break;
		}
	}
}

void CGameObject::Initialize()
{

}

void CGameObject::Reset(const XMFLOAT4X4& TransformMatrix)
{

}

void CGameObject::Animate(float ElapsedTime)
{

}

shared_ptr<CGameObject> CGameObject::FindFrame(const tstring& FrameName)
{
	shared_ptr<CGameObject> Object{};

	if (m_FrameName == FrameName)
	{
		return shared_from_this();
	}

	for (const auto& ChildObject : m_ChildObjects)
	{
		if (ChildObject)
		{
			if (Object = ChildObject->FindFrame(FrameName))
			{
				return Object;
			}
		}
	}

	return Object;
}

shared_ptr<CSkinnedMesh> CGameObject::FindSkinnedMesh(const tstring& SkinnedMeshName)
{
	shared_ptr<CSkinnedMesh> SkinnedMesh{};

	if (m_Mesh)
	{
		if (typeid(*m_Mesh) == typeid(CSkinnedMesh))
		{
			if (m_Mesh->GetName() == SkinnedMeshName)
			{
				return static_pointer_cast<CSkinnedMesh>(m_Mesh);
			}
		}
	}

	for (const auto& ChildObject : m_ChildObjects)
	{
		if (ChildObject)
		{
			if (SkinnedMesh = ChildObject->FindSkinnedMesh(SkinnedMeshName))
			{
				return SkinnedMesh;
			}
		}
	}

	return SkinnedMesh;
}

shared_ptr<CGameObject> CGameObject::PickObjectByRayIntersection(const XMFLOAT3& RayOrigin, const XMFLOAT3& RayDirection, float& HitDistance, float MaxDistance, bool NavPath)
{
	shared_ptr<CGameObject> NearestIntersectedObject{};

	if (m_Mesh && m_BoundingBox)
	{
		// 광선과 바운딩박스의 교차를 검사한다.
		bool IsIntersected{ m_BoundingBox->Intersects(XMLoadFloat3(&RayOrigin), XMLoadFloat3(&RayDirection), HitDistance) };

		if (IsIntersected)
		{
			// 광원과 바운딩 박스 사이의 거리가 MaxDistance보다 작다면 광선과 메쉬(삼각형)의 교차를 검사한다.
			if (HitDistance < MaxDistance)
			{
				if (NavPath)
				{
					if ((m_BoundingBox->Center.y + m_BoundingBox->Extents.y) < 6.0f)
					{
						return shared_from_this();
					}
				}
				if (m_Mesh->CheckRayIntersection(RayOrigin, RayDirection, XMLoadFloat4x4(&m_WorldMatrix), HitDistance))
				{
					return shared_from_this();
				}
			}
		}
	}

	float NearestHitDistance{ FLT_MAX };

	for (const auto& ChildObject : m_ChildObjects)
	{
		if (ChildObject)
		{
			shared_ptr<CGameObject> IntersectedObject = ChildObject->PickObjectByRayIntersection(RayOrigin, RayDirection, HitDistance, MaxDistance, NavPath);

			if (IntersectedObject && (HitDistance < NearestHitDistance))
			{
				NearestIntersectedObject = IntersectedObject;
				NearestHitDistance = HitDistance;
			}
		}
	}

	if (NearestIntersectedObject)
	{
		HitDistance = NearestHitDistance;
	}

	return NearestIntersectedObject;
}

void CGameObject::SetActive(bool IsActive)
{
	m_IsActive = IsActive;
}

bool CGameObject::IsActive() const
{
	return m_IsActive;
}

const tstring& CGameObject::GetName() const
{
	return m_FrameName;
}

const XMFLOAT4X4& CGameObject::GetWorldMatrix() const
{
	return m_WorldMatrix;
}

void CGameObject::SetTransformMatrix(const XMFLOAT4X4& TransformMatrix)
{
	m_TransformMatrix = TransformMatrix;
}

const XMFLOAT4X4& CGameObject::GetTransformMatrix() const
{
	return m_TransformMatrix;
}

void CGameObject::SetRight(const XMFLOAT3& Right)
{
	m_TransformMatrix._11 = Right.x;
	m_TransformMatrix._12 = Right.y;
	m_TransformMatrix._13 = Right.z;

	UpdateTransform(Matrix4x4::Identity());
}

XMFLOAT3 CGameObject::GetRight() const
{
	return XMFLOAT3(m_WorldMatrix._11, m_WorldMatrix._12, m_WorldMatrix._13);
}

void CGameObject::SetUp(const XMFLOAT3& Up)
{
	m_TransformMatrix._21 = Up.x;
	m_TransformMatrix._22 = Up.y;
	m_TransformMatrix._23 = Up.z;

	UpdateTransform(Matrix4x4::Identity());
}

XMFLOAT3 CGameObject::GetUp() const
{
	return XMFLOAT3(m_WorldMatrix._21, m_WorldMatrix._22, m_WorldMatrix._23);
}

void CGameObject::SetLook(const XMFLOAT3& Look)
{
	m_TransformMatrix._31 = Look.x;
	m_TransformMatrix._32 = Look.y;
	m_TransformMatrix._33 = Look.z;

	UpdateTransform(Matrix4x4::Identity());
}

XMFLOAT3 CGameObject::GetLook() const
{
	return XMFLOAT3(m_WorldMatrix._31, m_WorldMatrix._32, m_WorldMatrix._33);
}

void CGameObject::UpdateLocalCoord(const XMFLOAT3& LookDirection)
{
	const XMFLOAT3 WorldUp{ 0.0f, 1.0f, 0.0f };

	SetLook(LookDirection);
	SetRight(Vector3::CrossProduct(WorldUp, GetLook(), false));
	SetUp(Vector3::CrossProduct(GetLook(), GetRight(), false));
}

void CGameObject::SetPosition(const XMFLOAT3& Position)
{
	m_TransformMatrix._41 = Position.x;
	m_TransformMatrix._42 = Position.y;
	m_TransformMatrix._43 = Position.z;

	UpdateTransform(Matrix4x4::Identity());
}

XMFLOAT3 CGameObject::GetPosition() const
{
	return XMFLOAT3(m_WorldMatrix._41, m_WorldMatrix._42, m_WorldMatrix._43);
}

void CGameObject::SetMesh(const shared_ptr<CMesh>& Mesh)
{
	if (Mesh)
	{
		m_Mesh = Mesh;
	}
}

void CGameObject::SetAnimationController(const shared_ptr<LOADED_MODEL_INFO>& ModelInfo)
{
	if (ModelInfo)
	{
		m_AnimationController = make_shared<CAnimationController>(ModelInfo, shared_from_this());
	}
}

shared_ptr<CAnimationController> CGameObject::GetAnimationController() const
{
	return m_AnimationController;
}

void CGameObject::SetBoundingBox(const shared_ptr<BoundingBox>& BoundingBox)
{
	if (BoundingBox)
	{
		m_BoundingBox = BoundingBox;
	}
}

shared_ptr<BoundingBox> CGameObject::GetBoundingBox() const
{
	return m_BoundingBox;
}

void CGameObject::SetChild(const shared_ptr<CGameObject>& ChildObject)
{
	if (ChildObject)
	{
		m_ChildObjects.push_back(ChildObject);
	}
}

bool CGameObject::IsInNavMesh(const shared_ptr<CNavMesh>& NavMesh, XMFLOAT3& NewPosition)
{
	if (NavMesh)
	{
		for (const auto& NavNode : NavMesh->GetNavNodes())
		{
			TRIANGLE Triangle{ NavNode->GetTriangle() };

			if (Math::IsInTriangle(Triangle.m_Vertices[0], Triangle.m_Vertices[1], Triangle.m_Vertices[2], NewPosition))
			{
				// NavMesh 안에 있다면, NewPosition의 높이 값을 밟고 있는 삼각형의 무게중심 높이로 설정한다.
				NewPosition.y = Triangle.m_Centroid.y;

				return true;
			}
		}
	}

	return false;
}

void CGameObject::UpdateBoundingBox()
{
	if (m_Mesh && m_BoundingBox)
	{
		m_Mesh->GetBoundingBox().Transform(*m_BoundingBox, XMLoadFloat4x4(&m_WorldMatrix));
	}
}

void CGameObject::UpdateTransform(const XMFLOAT4X4& ParentMatrix)
{
	m_WorldMatrix = Matrix4x4::Multiply(m_TransformMatrix, ParentMatrix);

	UpdateBoundingBox();

	for (const auto& ChildObject : m_ChildObjects)
	{
		if (ChildObject)
		{
			ChildObject->UpdateTransform(m_WorldMatrix);
		}
	}
}

void CGameObject::Move(const XMFLOAT3& Direction, float Distance)
{
	XMFLOAT3 Shift{ Vector3::ScalarProduct(Distance, Direction, false) };

	SetPosition(Vector3::Add(GetPosition(), Shift));
}

void CGameObject::Scale(float Pitch, float Yaw, float Roll)
{
	XMFLOAT4X4 ScaleMatrix{ Matrix4x4::Scale(Pitch, Yaw, Roll) };

	m_TransformMatrix = Matrix4x4::Multiply(ScaleMatrix, m_TransformMatrix);

	UpdateTransform(Matrix4x4::Identity());
}

void CGameObject::Rotate(float Pitch, float Yaw, float Roll)
{
	XMFLOAT4X4 RotationMatrix{ Matrix4x4::RotationYawPitchRoll(Pitch, Yaw, Roll) };

	m_TransformMatrix = Matrix4x4::Multiply(RotationMatrix, m_TransformMatrix);

	UpdateTransform(Matrix4x4::Identity());
}

void CGameObject::Rotate(const XMFLOAT3& Axis, float Angle)
{
	XMFLOAT4X4 RotationMatrix{ Matrix4x4::RotationAxis(Axis, Angle) };

	m_TransformMatrix = Matrix4x4::Multiply(RotationMatrix, m_TransformMatrix);

	UpdateTransform(Matrix4x4::Identity());
}
