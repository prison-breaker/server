#pragma once
#pragma comment(lib, "ws2_32")

#include "targetver.h"

// 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#define WIN32_LEAN_AND_MEAN      
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define EPSILON			     1.0e-10f

#define MAX_CLIENT_CAPACITY  2
#define MAX_NPC_COUNT		 15

#define SERVER_PORT          9000

#define INPUT_MASK_NONE		 0x0000
#define INPUT_MASK_W	     0x0001
#define INPUT_MASK_S         0x0002
#define INPUT_MASK_A         0x0004
#define INPUT_MASK_D         0x0008
#define INPUT_MASK_F		 0x0010
#define INPUT_MASK_SHIFT     0x0020
#define INPUT_MASK_TAB       0x0040
#define INPUT_MASK_LMB       0x0080
#define INPUT_MASK_RMB       0x0100

enum SCENE_TYPE
{
	SCENE_TYPE_TITLE,
	SCENE_TYPE,INGAME
};

enum OBJECT_TYPE
{
	OBJECT_TYPE_PLAYER,
	OBJECT_TYPE_NPC,
	OBJECT_TYPE_TERRAIN,
	OBJECT_TYPE_STRUCTURE
};

enum WEAPON_TYPE
{
	WEAPON_TYPE_PUNCH,
	WEAPON_TYPE_PISTOL
};

enum ANIMATION_TYPE
{
	ANIMATION_TYPE_LOOP,
	ANIMATION_TYPE_ONCE,
	ANIMATION_TYPE_ONCE_REVERSE
};

enum ANIMATION_CLIP_TYPE
{
	ANIMATION_CLIP_TYPE_PLAYER_IDLE = 0,
	ANIMATION_CLIP_TYPE_PLAYER_WALK_FORWARD_AND_BACK,
	ANIMATION_CLIP_TYPE_PLAYER_WALK_LEFT,
	ANIMATION_CLIP_TYPE_PLAYER_WALK_RIGHT,
	ANIMATION_CLIP_TYPE_PLAYER_RUN_FORWARD,
	ANIMATION_CLIP_TYPE_PLAYER_RUN_LEFT,
	ANIMATION_CLIP_TYPE_PLAYER_RUN_RIGHT,
	ANIMATION_CLIP_TYPE_PLAYER_PUNCH,
	ANIMATION_CLIP_TYPE_PLAYER_PISTOL_IDLE,
	ANIMATION_CLIP_TYPE_PLAYER_SHOOT,
	ANIMATION_CLIP_TYPE_PLAYER_DIE,

	ANIMATION_CLIP_TYPE_NPC_IDLE = 0,
	ANIMATION_CLIP_TYPE_NPC_WALK_FORWARD,
	ANIMATION_CLIP_TYPE_NPC_RUN_FORWARD,
	ANIMATION_CLIP_TYPE_NPC_SHOOT,
	ANIMATION_CLIP_TYPE_NPC_HIT,
	ANIMATION_CLIP_TYPE_NPC_DIE
};

// Windows 헤더 파일
#include <winsock2.h>

// C 런타임 헤더 파일입니다.
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// C++ 런타임 헤더 파일입니다.
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
using namespace std;

// DirectX Header
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
using namespace DirectX;
using namespace DirectX::PackedVector;

#ifdef _UNICODE
#define tcout wcout
#define tcin  wcin
#else
#define tcout cout
#define tcin  cin
#endif

typedef basic_string<TCHAR>        tstring;
typedef basic_istream<TCHAR>       tistream;
typedef basic_ostream<TCHAR>       tostream;
typedef basic_fstream<TCHAR>       tfstream;
typedef basic_ifstream<TCHAR>      tifstream;
typedef basic_ofstream<TCHAR>      tofstream;
typedef basic_stringstream<TCHAR>  tstringstream;
typedef basic_istringstream<TCHAR> tistringstream;
typedef basic_ostringstream<TCHAR> tostringstream;

struct SOCKET_INFO
{
	UINT		m_ID{};
	SOCKET      m_Socket{};
	SOCKADDR_IN m_SocketAddress{};

	bool	    m_Completed{};
};

struct LIGHT
{
	bool	   m_IsActive{};

	XMFLOAT3   m_Position{};
	XMFLOAT3   m_Direction{};

	float      m_SpotLightAngle{};
};

struct CLIENT_TO_SERVER_DATA
{
	UINT	   m_SceneState{};

	UINT	   m_InputMask{};
	XMFLOAT4X4 m_WorldMatrix{};
};

struct SERVER_TO_CLIENT_DATA
{
	XMFLOAT4X4          m_PlayerWorldMatrices[MAX_CLIENT_CAPACITY]{};
	ANIMATION_CLIP_TYPE m_PlayerAnimationClipTypes[MAX_CLIENT_CAPACITY]{};

	XMFLOAT4X4          m_NPCWorldMatrices[MAX_NPC_COUNT]{};
	ANIMATION_CLIP_TYPE m_NPCAnimationClipTypes[MAX_NPC_COUNT]{};

	XMFLOAT3            m_TowerLightDirection{};

	// 1. Scene's State - UINT
	// 4. UI and Trigger's Activation Condition - bool
	// 6. Sound Play Condition and Volume - bool and float
};

namespace File
{
	UINT ReadIntegerFromFile(tifstream& InFile);
	float ReadFloatFromFile(tifstream& InFile);
	void ReadStringFromFile(tifstream& InFile, tstring& Token);
}

namespace Random
{
	float Range(float Min, float Max);
}

namespace Math
{
	bool IsZero(float Value);
	bool IsEqual(float Value1, float Value2);

	float InverseSqrt(float Value);

	float Distance(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2);

	UINT CalculateTriangleArea(const XMFLOAT3& Vertex1, const XMFLOAT3& Vertex2, const XMFLOAT3& Vertex3);
	bool IsInTriangle(const XMFLOAT3& Vertex1, const XMFLOAT3& Vertex2, const XMFLOAT3& Vertex3, const XMFLOAT3& NewPosition);

	int CounterClockWise(const XMFLOAT3& Vertex1, const XMFLOAT3& Vertex2, const XMFLOAT3& Vertex3);
	bool LineIntersection(const XMFLOAT3& L1V1, const XMFLOAT3& L1V2, const XMFLOAT3& L2V1, const XMFLOAT3& L2V2);
}

namespace Vector3
{
	bool IsZero(const XMFLOAT3& Vector);

	bool IsEqual(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2);

	float Length(const XMFLOAT3& Vector);
	XMFLOAT3 Normalize(const XMFLOAT3& Vector);
	XMFLOAT3 Inverse(const XMFLOAT3& Vector);

	XMFLOAT3 XMVectorToXMFloat3(const XMVECTOR& Vector);

	XMFLOAT3 Add(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2);
	XMFLOAT3 Subtract(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2);

	XMFLOAT3 ScalarProduct(float Scalar, const XMFLOAT3& Vector, bool Normalize);
	float DotProduct(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2);
	XMFLOAT3 CrossProduct(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2, bool Normalize);

	float Angle(const XMVECTOR& Vector1, const XMVECTOR& Vector2);
	float Angle(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2);

	XMFLOAT3 TransformNormal(const XMFLOAT3& Vector, const XMMATRIX& Matrix);
	XMFLOAT3 TransformNormal(const XMFLOAT3& Vector, const XMFLOAT4X4& Matrix);
	XMFLOAT3 TransformCoord(const XMFLOAT3& Vector, const XMMATRIX& Matrix);
	XMFLOAT3 TransformCoord(const XMFLOAT3& Vector, const XMFLOAT4X4& Matrix);
}

namespace Matrix4x4
{
	XMFLOAT4X4 Identity();
	XMFLOAT4X4 Inverse(const XMFLOAT4X4& Matrix);
	XMFLOAT4X4 Transpose(const XMFLOAT4X4& Matrix);

	XMFLOAT4X4 Multiply(const XMFLOAT4X4& Matrix1, const XMFLOAT4X4& Matrix2);
	XMFLOAT4X4 Multiply(const XMFLOAT4X4& Matrix1, const XMMATRIX& Matrix2);
	XMFLOAT4X4 Multiply(const XMMATRIX& Matrix1, const XMFLOAT4X4& Matrix2);

	XMFLOAT4X4 Scale(float Pitch, float Yaw, float Roll);

	XMFLOAT4X4 RotationYawPitchRoll(float Pitch, float Yaw, float Roll);
	XMFLOAT4X4 RotationAxis(const XMFLOAT3& Axis, float Angle);

	XMFLOAT4X4 OrthographicFovLH(float Width, float Height, float NearZ, float FarZ);
	XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ);

	XMFLOAT4X4 LookAtLH(const XMFLOAT3& Position, const XMFLOAT3& FocusPosition, const XMFLOAT3& UpDirection);
	XMFLOAT4X4 LookToLH(const XMFLOAT3& Position, const XMFLOAT3& Look, const XMFLOAT3& WorldUp);
}

namespace Server
{
	void ErrorQuit(const char* Msg);
	void ErrorDisplay(const char* Msg);
}
