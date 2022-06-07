#include "stdafx.h"

namespace File
{
	UINT ReadIntegerFromFile(tifstream& InFile)
	{
		UINT Value{};

		InFile.read(reinterpret_cast<TCHAR*>(&Value), sizeof(int));

		return Value;
	}

	float ReadFloatFromFile(tifstream& InFile)
	{
		float Value{};

		InFile.read(reinterpret_cast<TCHAR*>(&Value), sizeof(float));

		return Value;
	}

	void ReadStringFromFile(tifstream& InFile, tstring& Token)
	{
		UINT Length{};

		InFile.read(reinterpret_cast<TCHAR*>(&Length), sizeof(BYTE));
		Token.resize(Length);
		InFile.read(reinterpret_cast<TCHAR*>(&Token[0]), sizeof(BYTE) * Length);
	}
}

namespace Random
{
	float Range(float Min, float Max)
	{
		return Min + (Max - Min) * (static_cast<float>(rand()) / (RAND_MAX));
	}
}

namespace Math
{
	bool IsZero(float Value)
	{
		return fabsf(Value) < EPSILON;
	}

	bool IsEqual(float Value1, float Value2)
	{
		return IsZero(Value1 - Value2);
	}

	float InverseSqrt(float Value)
	{
		return 1.0f / sqrtf(Value);
	}

	float Distance(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2)
	{
		return sqrtf(powf(Vector2.x - Vector1.x, 2) + powf(Vector2.y - Vector1.y, 2) + powf(Vector2.z - Vector1.z, 2));
	}

	UINT CalculateTriangleArea(const XMFLOAT3& Vertex1, const XMFLOAT3& Vertex2, const XMFLOAT3& Vertex3)
	{
		UINT Area{ static_cast<UINT>(abs((Vertex1.x * (Vertex2.z - Vertex3.z)) + (Vertex2.x * (Vertex3.z - Vertex1.z)) + (Vertex3.x * (Vertex1.z - Vertex2.z)))) };

		return Area;
	}

	bool IsInTriangle(const XMFLOAT3& Vertex1, const XMFLOAT3& Vertex2, const XMFLOAT3& Vertex3, const XMFLOAT3& NewPosition)
	{
		XMFLOAT3 Vertex1ToPosition{ Vector3::Subtract(NewPosition, Vertex1) };
		XMFLOAT3 Vertex2ToPosition{ Vector3::Subtract(NewPosition, Vertex2) };
		XMFLOAT3 Vertex3ToPosition{ Vector3::Subtract(NewPosition, Vertex3) };

		XMFLOAT3 Vertex1ToVertex2{ Vector3::Subtract(Vertex2, Vertex1) };
		XMFLOAT3 Vertex2ToVertex3{ Vector3::Subtract(Vertex3, Vertex2) };
		XMFLOAT3 Vertex3ToVertex1{ Vector3::Subtract(Vertex1, Vertex3) };

		return (Vector3::CrossProduct(Vertex1ToVertex2, Vertex1ToPosition, false).y > 0.0f) &&
			(Vector3::CrossProduct(Vertex2ToVertex3, Vertex2ToPosition, false).y > 0.0f) &&
			(Vector3::CrossProduct(Vertex3ToVertex1, Vertex3ToPosition, false).y > 0.0f);
	}

	int CounterClockWise(const XMFLOAT3& Vertex1, const XMFLOAT3& Vertex2, const XMFLOAT3& Vertex3)
	{
		// Vertex1과 Vertex2를 이은 선분과, Vertex1과 Vertex3을 이은 선분의 기울기를 구하고 양변 모두 분모를 없앤 후 좌변으로 이항하면 아래와 같이 CCW를 판별할 수 있는 공식이 나온다.
		float GradientDiff{ (Vertex2.x - Vertex1.x) * (Vertex3.z - Vertex1.z) - (Vertex3.x - Vertex1.x) * (Vertex2.z - Vertex1.z) };

		if (GradientDiff < 0.0f)
		{
			return -1;
		}
		else if (GradientDiff > 0.0f)
		{
			return 1;
		}

		return 0;
	}

	bool LineIntersection(const XMFLOAT3& L1V1, const XMFLOAT3& L1V2, const XMFLOAT3& L2V1, const XMFLOAT3& L2V2)
	{
		int L1_L2{ CounterClockWise(L1V1, L1V2, L2V1) * CounterClockWise(L1V1, L1V2, L2V2) };
		int L2_L1{ CounterClockWise(L2V1, L2V2, L1V1) * CounterClockWise(L2V1, L2V2, L1V2) };

		return (L1_L2 <= 0) && (L2_L1 <= 0);
	}
}

namespace Vector3
{
	bool IsZero(const XMFLOAT3& Vector)
	{
		if (Math::IsZero(Vector.x) && Math::IsZero(Vector.y) && Math::IsZero(Vector.z))
		{
			return true;
		}

		return false;
	}

	bool IsEqual(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2)
	{
		if (Math::IsEqual(Vector1.x, Vector2.x) && Math::IsEqual(Vector1.y, Vector2.y) && Math::IsEqual(Vector1.z, Vector2.z))
		{
			return true;
		}

		return false;
	}

	float Length(const XMFLOAT3& Vector)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, XMVector3Length(XMLoadFloat3(&Vector)));

		return Result.x;
	}

	XMFLOAT3 Normalize(const XMFLOAT3& Vector)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, XMVector3Normalize(XMLoadFloat3(&Vector)));

		return Result;
	}

	XMFLOAT3 Inverse(const XMFLOAT3& Vector)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, XMVectorScale(XMLoadFloat3(&Vector), -1.0f));

		return Result;
	}

	XMFLOAT3 XMVectorToXMFloat3(const XMVECTOR& Vector)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, Vector);

		return Result;
	}

	XMFLOAT3 Add(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, XMLoadFloat3(&Vector1) + XMLoadFloat3(&Vector2));

		return Result;
	}

	XMFLOAT3 Subtract(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, XMLoadFloat3(&Vector1) - XMLoadFloat3(&Vector2));

		return Result;
	}

	XMFLOAT3 ScalarProduct(float Scalar, const XMFLOAT3& Vector, bool Normalize)
	{
		XMFLOAT3 Result{};

		if (Normalize)
		{
			XMStoreFloat3(&Result, XMVector3Normalize(Scalar * XMLoadFloat3(&Vector)));
		}
		else
		{
			XMStoreFloat3(&Result, XMLoadFloat3(&Vector) * Scalar);
		}

		return Result;
	}

	float DotProduct(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, XMVector3Dot(XMLoadFloat3(&Vector1), XMLoadFloat3(&Vector2)));

		return Result.x;
	}

	XMFLOAT3 CrossProduct(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2, bool Normalize)
	{
		XMFLOAT3 Result{};

		if (Normalize)
		{
			XMStoreFloat3(&Result, XMVector3Normalize(XMVector3Cross(XMLoadFloat3(&Vector1), XMLoadFloat3(&Vector2))));
		}
		else
		{
			XMStoreFloat3(&Result, XMVector3Cross(XMLoadFloat3(&Vector1), XMLoadFloat3(&Vector2)));
		}

		return Result;
	}

	float Angle(const XMVECTOR& Vector1, const XMVECTOR& Vector2)
	{
		XMVECTOR Result{ XMVector3AngleBetweenNormals(Vector1, Vector2) };

		return XMConvertToDegrees(XMVectorGetX(Result));
	}

	float Angle(const XMFLOAT3& Vector1, const XMFLOAT3& Vector2)
	{
		return Angle(XMLoadFloat3(&Vector1), XMLoadFloat3(&Vector2));
	}

	XMFLOAT3 TransformNormal(const XMFLOAT3& Vector, const XMMATRIX& Matrix)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, XMVector3TransformNormal(XMLoadFloat3(&Vector), Matrix));

		return Result;
	}

	XMFLOAT3 TransformNormal(const XMFLOAT3& Vector, const XMFLOAT4X4& Matrix)
	{
		return TransformNormal(Vector, XMLoadFloat4x4(&Matrix));
	}

	XMFLOAT3 TransformCoord(const XMFLOAT3& Vector, const XMMATRIX& Matrix)
	{
		XMFLOAT3 Result{};

		XMStoreFloat3(&Result, XMVector3TransformCoord(XMLoadFloat3(&Vector), Matrix));

		return Result;
	}

	XMFLOAT3 TransformCoord(const XMFLOAT3& Vector, const XMFLOAT4X4& Matrix)
	{
		return TransformCoord(Vector, XMLoadFloat4x4(&Matrix));
	}
}

namespace Matrix4x4
{
	XMFLOAT4X4 Identity()
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixIdentity());

		return Result;
	}

	XMFLOAT4X4 Inverse(const XMFLOAT4X4& Matrix)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixInverse(nullptr, XMLoadFloat4x4(&Matrix)));

		return Result;
	}

	XMFLOAT4X4 Transpose(const XMFLOAT4X4& Matrix)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixTranspose(XMLoadFloat4x4(&Matrix)));

		return Result;
	}

	XMFLOAT4X4 Multiply(const XMFLOAT4X4& Matrix1, const XMFLOAT4X4& Matrix2)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMLoadFloat4x4(&Matrix1) * XMLoadFloat4x4(&Matrix2));

		return Result;
	}

	XMFLOAT4X4 Multiply(const XMFLOAT4X4& Matrix1, const XMMATRIX& Matrix2)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMLoadFloat4x4(&Matrix1) * Matrix2);

		return Result;
	}

	XMFLOAT4X4 Multiply(const XMMATRIX& Matrix1, const XMFLOAT4X4& Matrix2)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, Matrix1 * XMLoadFloat4x4(&Matrix2));

		return Result;
	}

	XMFLOAT4X4 Scale(float Pitch, float Yaw, float Roll)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixScaling(Pitch, Yaw, Roll));

		return Result;
	}

	XMFLOAT4X4 RotationYawPitchRoll(float Pitch, float Yaw, float Roll)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixRotationRollPitchYaw(XMConvertToRadians(Pitch), XMConvertToRadians(Yaw), XMConvertToRadians(Roll)));

		return Result;
	}

	XMFLOAT4X4 RotationAxis(const XMFLOAT3& Axis, float Angle)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixRotationAxis(XMLoadFloat3(&Axis), XMConvertToRadians(Angle)));

		return Result;
	}

	XMFLOAT4X4 OrthographicFovLH(float ViewWidth, float ViewHeight, float NearZ, float FarZ)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixOrthographicLH(ViewWidth, ViewHeight, NearZ, FarZ));

		return Result;
	}

	XMFLOAT4X4 PerspectiveFovLH(float FovAngleY, float AspectRatio, float NearZ, float FarZ)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));

		return Result;
	}

	XMFLOAT4X4 LookAtLH(const XMFLOAT3& Position, const XMFLOAT3& FocusPosition, const XMFLOAT3& UpDirection)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixLookAtLH(XMLoadFloat3(&Position), XMLoadFloat3(&FocusPosition), XMLoadFloat3(&UpDirection)));

		return Result;
	}

	XMFLOAT4X4 LookToLH(const XMFLOAT3& Position, const XMFLOAT3& Look, const XMFLOAT3& WorldUp)
	{
		XMFLOAT4X4 Result{};

		XMStoreFloat4x4(&Result, XMMatrixLookToLH(XMLoadFloat3(&Position), XMLoadFloat3(&Look), XMLoadFloat3(&WorldUp)));

		return Result;
	}
}

namespace Server
{
	void ErrorQuit(const char* Msg)
	{
		LPVOID MsgBuffer{};

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&MsgBuffer, 0, NULL);
		MessageBox(NULL, (LPCTSTR)MsgBuffer, (LPCTSTR)Msg, MB_ICONERROR);

		LocalFree(MsgBuffer);
		exit(1);
	}

	void ErrorDisplay(const char* Msg)
	{
		LPVOID MsgBuffer{};

		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&MsgBuffer, 0, NULL);
		cout << "[" << Msg << "] " << (char*)MsgBuffer;

		LocalFree(MsgBuffer);
	}
}
