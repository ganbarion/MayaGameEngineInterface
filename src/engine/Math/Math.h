//
// Copyright (c) GANBARION Co., Ltd. All rights reserved.
// This code is licensed under the MIT License (MIT).
//

#pragma once 

#include <DirectXMath.h>
using namespace DirectX;

namespace se
{
	/**
	 * Vector3
	 */
	struct Vector3 : public XMFLOAT3
	{
		Vector3(){}
		Vector3(float xyz)
			: XMFLOAT3(xyz, xyz, xyz)
		{
		}
		Vector3(float x, float y, float z)
			: XMFLOAT3(x, y, z)
		{
		}
		Vector3(const XMFLOAT3& xyz)
			: XMFLOAT3(xyz)
		{
		}
		Vector3(FXMVECTOR v)
		{
			XMStoreFloat3(this, v);
		}

		Vector3& operator*=(const Vector3& other)
		{
			x *= other.x;
			y *= other.y;
			z *= other.z;
			return *this;
		}
		Vector3 operator*(const Vector3& other) const
		{
			Vector3 result;
			result.x = x * other.x;
			result.y = y * other.y;
			result.z = z * other.z;
			return result;
		}
		Vector3& operator*=(float s)
		{
			x *= s;
			y *= s;
			z *= s;
			return *this;
		}
		Vector3 operator*(float s) const
		{
			Vector3 result;
			result.x = x * s;
			result.y = y * s;
			result.z = z * s;
			return result;
		}

		float* ToFloatArray() { return reinterpret_cast<float*>(this); }
		const float* ToFloatArray() const { return reinterpret_cast<const float*>(this); }
	};
	typedef Vector3 float3;


	/**
	 * Vector4
	 */
	struct Vector4 : public XMFLOAT4
	{
		Vector4(){}
		Vector4(float xyz)
			: XMFLOAT4(xyz, xyz, xyz, 1.0f)
		{
		}
		Vector4(float x, float y, float z, float w)
			: XMFLOAT4(x, y, z, w)
		{
		}
		Vector4(const XMFLOAT4& xyzw)
			: XMFLOAT4(xyzw)
		{
		}
		Vector4(const XMFLOAT3& xyz)
		{
			x = xyz.x;
			y = xyz.y;
			z = xyz.z;
			w = 1.0f;
		}
		Vector4(FXMVECTOR v)
		{
			XMStoreFloat4(this, v);
		}

		float* ToFloatArray() { return reinterpret_cast<float*>(this); }
		const float* ToFloatArray() const { return reinterpret_cast<const float*>(this); }
	};
	typedef Vector4 float4;


	/**
	 * Matrix4x4
	 */
	struct Matrix44 : public XMFLOAT4X4
	{
		Matrix44()
		{
		}
		Matrix44(const XMFLOAT4X4& m)
			: XMFLOAT4X4(m)
		{
		}
		Matrix44(CXMMATRIX m)
		{
			XMStoreFloat4x4(this, m);
		}

		Matrix44& operator*=(const Matrix44& other)
		{
			auto result = ToMatrix() * other.ToMatrix();
			XMStoreFloat4x4(this, result);
			return *this;
		}
		Matrix44 operator*(const Matrix44& other) const
		{
			auto result = ToMatrix() * other.ToMatrix();
			return Matrix44(result);
		}

		bool operator==(const Matrix44& other) const
		{
			return _11 == other._11 && _12 == other._12 && _13 == other._13 && _14 == other._14
				&& _21 == other._21 && _22 == other._22 && _23 == other._23 && _24 == other._24
				&& _31 == other._31 && _32 == other._32 && _33 == other._33 && _34 == other._34
				&& _41 == other._41 && _42 == other._42 && _43 == other._43 && _44 == other._44;
		}
		bool operator!=(const Matrix44& other) const
		{
			return !(*this == other);
		}

		void Ident()
		{
			_11 = _22 = _33 = _44 = 1.00f;
			_12 = _13 = _14 = 0.0f;
			_21 = _23 = _24 = 0.0f;
			_31 = _32 = _34 = 0.0f;
			_41 = _42 = _43 = 0.0f;
		}

		Vector3 Translation() const
		{
			return Vector3(_41, _42, _43);
		}

		void SetTranslation(const Vector3& t)
		{
			_41 = t.x;
			_42 = t.y;
			_43 = t.z;
		}

		XMMATRIX ToMatrix() const { return XMLoadFloat4x4(this); }

		/* static methods */
		static Matrix44 Transpose(const Matrix44& m)
		{
			return XMMatrixTranspose(m.ToMatrix());
		}

		static Matrix44 Invert(const Matrix44& m)
		{
			XMVECTOR det;
			return XMMatrixInverse(&det, m.ToMatrix());
		}

		static Matrix44 ScaleMatrix(float s)
		{
			Matrix44 m;
			m.Ident();
			m._11 = s;
			m._22 = s;
			m._33 = s;
			return m;
		}

		static Matrix44 ScaleMatrix(const Vector3& s)
		{
			Matrix44 m;
			m.Ident();
			m._11 = s.x;
			m._22 = s.y;
			m._33 = s.z;
			return m;
		}

		static Matrix44 TranslationMatrix(const Vector3& t)
		{
			Matrix44 m;
			m.Ident();
			m._41 = t.x;
			m._42 = t.y;
			m._43 = t.z;
			return m;
		}
	};
	typedef Matrix44 float4x4;


	/**
	 * Rect
	 */
	template <typename T>
	struct TRect
	{
		T x;
		T y;
		T width;
		T height;

		TRect()
		{
		}
		TRect(T x, T y, T width, T height)
			: x(x)
			, y(y)
			, width(width)
			, height(height)
		{
		}

		void Set(T x, T y, T width, T height)
		{
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}
	};
	typedef TRect<int32_t> Rect;
	typedef TRect<float> Rectf;


	template<typename T>
	__forceinline T Min(T a, T b)
	{
		return (a < b) ? a : b;
	}

	template<typename T>
	__forceinline T Max(T a, T b)
	{
		return (a < b) ? b : a;
	}

	template<typename T> 
	__forceinline T Clamp(T val, T min, T max)
	{
		if (val < min) val = min;
		else if (val > max) val = max;
		return val;
	}
}