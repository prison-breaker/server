#pragma once

struct LOADED_MODEL_INFO;
class CGameObject;
class CSkinnedMesh;

class CAnimationClip
{
	friend class CAnimationController;

private:
	string                             m_ClipName{};
                             
	UINT	                           m_FramePerSec{};
	UINT	                           m_KeyFrameCount{};
	float                              m_KeyFrameTime{};

	vector<vector<vector<XMFLOAT4X4>>> m_BoneTransformMatrixes{}; // [SkinnedMesh][Bone][KeyFrameTimeIndex]

public:
	CAnimationClip() = default;
	~CAnimationClip() = default;

	void LoadAnimationClipInfoFromFile(ifstream& InFile, const shared_ptr<LOADED_MODEL_INFO>& ModelInfo);
};

//=========================================================================================================================

class CAnimationController
{
private:
	bool									m_IsActive{ true };

	float									m_ElapsedTime{};

	shared_ptr<CGameObject>					m_Owner{};

	ANIMATION_CLIP_TYPE						m_ClipType{};
	vector<shared_ptr<CAnimationClip>>	    m_AnimationClips{};

	UINT							        m_KeyFrameIndex{};

	vector<shared_ptr<CSkinnedMesh>>        m_SkinnedMeshCaches{};
	vector<vector<shared_ptr<CGameObject>>> m_BoneFrameCaches{}; // [SkinnedMesh][Bone]

public:
	CAnimationController(const shared_ptr<LOADED_MODEL_INFO>& ModelInfo, const shared_ptr<CGameObject>& Owner);
	~CAnimationController() = default;
	
	void SetActive(bool IsActive);
	bool IsActive() const;

	void SetAnimationClipType(ANIMATION_CLIP_TYPE ClipType);
	ANIMATION_CLIP_TYPE GetAnimationClipType() const;

	void SetKeyFrameIndex(UINT KeyFrameIndex);
	UINT GetKeyFrameIndex() const;

	void UpdateShaderVariables();

	bool UpdateAnimationClip(ANIMATION_TYPE AnimationType);
};
