#pragma once

#include <cmath>
#include <immintrin.h>

namespace gmath
{
	template<typename T = float> struct Vec2;
	template<typename T = float> struct Vec3;
	template<typename T = float> struct Vec4;
	template<typename T = float> struct Mat3x3;
	template<typename T = float> struct Mat4x4;
	template<typename T = float> struct Quaternions;

	template<typename T>
	struct Vec4
	{
		union
		{
			struct
			{
				T x;
				T y;
				T z;
				T w;
			};
			struct
			{
				T r;
				T g;
				T b;
				T a;
			};
		};

		Vec4(T v = 0);
		Vec4(T x, T y, T z, T w = 0);
		Vec4(const Vec2<T>& vec2, T z = 0, T w = 0);
		Vec4(const Vec3<T>& vec3, T w = 0);
		Vec4(const Vec4& vec4) = default;

		T  Dot(const Vec4& rhs) const;

		Vec4 operator+(const Vec4& rhs) const;
		Vec4 operator-(const Vec4& rhs) const;

		Vec4& operator+=(const Vec4& rhs);
		Vec4& operator-=(const Vec4& rhs);
		Vec4& operator*=(const Vec4& rhs);
		Vec4& operator/=(const Vec4& rhs);

		template<typename T>
		friend Vec4<T> operator*(const Vec4<T>& lhs, T rhs);
		template<typename T>
		friend Vec4<T> operator*(T lhs, const Vec4<T>& rhs);
		template<typename T>
		friend Vec4<T> operator/(const Vec4<T>& lhs, T rhs);
		template<typename T>
		friend Vec4<T> operator*(const Vec4<T>& lhs, const Vec4<T>& rhs);
	};

	template<typename T>
	struct Vec3
	{
		T x;
		T y;
		T z;

		Vec3(T v = 0);
		Vec3(T x, T y, T z);
		Vec3(const Vec4<T>& vec4);
		Vec3(const Vec2<T>& vec2);
		Vec3(const Vec3& vec3) = default;

		Vec4<T> ToHomoCoord() const;
		T Dot(const Vec3& rhs) const;
		Vec3 Cross(const Vec3& b) const;
		Vec3 Normalize() const;
		T Length() const;
		Vec3 Reflect(Vec3 normal) const;
		Vec3 Pow(T rhs) const;
		Vec3 Sqrt() const;
		Quaternions<T> EularAngleToQuaternions() const;

		Vec3 operator+(const Vec3& rhs) const;
		Vec3 operator-(const Vec3& rhs) const;

		template<typename T>
		friend Vec3<T> operator*(const Vec3<T>& lhs, T rhs);
		template<typename T>
		friend Vec3<T> operator*(T lhs, const Vec3<T>& rhs);
		template<typename T>
		friend Vec3<T> operator/(const Vec3<T>& lhs, T rhs);
		template<typename T>
		friend Vec3<T> operator*(const Vec3<T>& lhs, const Vec3<T>& rhs);

		Vec3& operator+=(const Vec3& rhs);
		Vec3& operator-=(const Vec3& rhs);
		Vec3& operator*=(const Vec3& rhs);
		Vec3& operator/=(const Vec3& rhs);
	};

	template<typename T>
	struct Vec2
	{
		T x;
		T y;

		constexpr Vec2(T v = 0);
		constexpr Vec2(T x, T y);
		Vec2(const Vec4<T>& vec4);
		Vec2(const Vec3<T>& vec3);
		Vec2(const Vec2& vec2) = default;

		Vec2 operator+(const Vec2& rhs) const;
		Vec2 operator-(const Vec2& rhs) const;

		T Dot(const Vec2& rhs) const;
		T Cross(const Vec2 b) const;
		T Length() const;
		Vec2 Normalize() const;

		template<typename T>
		friend Vec2<T> operator*(const Vec2<T>& lhs, T rhs);
		template<typename T>
		friend Vec2<T> operator*(T lhs, const Vec2<T>& rhs);
		template<typename T>
		friend Vec2<T> operator/(const Vec2<T>& lhs, T rhs);
		template<typename T>
		friend Vec2<T> operator*(const Vec2<T>& lhs, const Vec2<T>& rhs);

		Vec2& operator+=(const Vec2& rhs);
		Vec2& operator-=(const Vec2& rhs);
		Vec2& operator*=(const Vec2& rhs);
		Vec2& operator/=(const Vec2& rhs);
	};

	template<typename T>
	struct Mat3x3
	{
		T data[9];

		Mat3x3() = default;
		Mat3x3(Vec3<T> a, Vec3<T> b, Vec3<T> c);
		Mat3x3(T a, T b, T c, T d, T e, T f, T g, T h, T i);

		//乘矩阵
		Mat3x3 operator*(const Mat3x3& rhs) const;
		//乘向量
		Vec3<T> operator*(const Vec3<T>& rhs) const;
		Vec4<T> operator*(const Vec4<T>& rhs) const;

		//构造单位矩阵
		static Mat3x3 Unit();
		//转置
		Mat3x3 Transpose() const;
		//求逆
		Mat3x3 Inverse() const;
	};

	template<typename T>
	struct Mat4x4
	{
		T data[16];

		//矩阵乘法
		Mat4x4 operator*(const Mat4x4& rhs) const;

		//乘向量 4分量
		Vec4<T> operator*(const Vec4<T>& rhs) const;

		Mat3x3<T> ToMat3x3() const;
	};

	template<typename T>
	struct Quaternions
	{
		T x;
		T y;
		T z;
		T w;

		Quaternions();
		Quaternions(Vec4<T>);
		Quaternions(Vec3<T>);
		Quaternions(Vec3<T>, float);
		Quaternions(float, float, float, float);

		//转换为欧拉角
		Vec3<T> ToEulerAngles() const;

		//转换为旋转矩阵
		Mat4x4<T> ToMat4() const;

		//转换为旋转矩阵
		Mat3x3<T> ToMat3() const;

		//乘四元数
		Quaternions operator*(const Quaternions& rhs) const;
		//归一化
		Quaternions Normalize() const;
		//求逆
		Quaternions Inverse() const;
		//乘向量
		Vec4<T> operator*(const Vec4<T>& rhs) const;
		//线性插值
		Quaternions Lerp(Quaternions rhs, float t) const;
		//球面插值
		Quaternions SLerp(Quaternions rhs, float t) const;
	};

	namespace utility
	{
		static constexpr float pi = 3.1415926f;
		template<typename T, typename U, typename V>
		T Clamp(T v, U a, V b);
		template<typename T>
		T Clamp(T v);
		template<typename T, typename U>
		T Lerp(T a, T b, U n);
		template<typename T>
		T BlendColor(T color0, T color1);
		float radians(float degree);
		float degrees(float radian);

		//单位矩阵
		template<typename T>
		static Mat3x3<T> Mat3Unit();
		template<typename T>
		static Mat4x4<T> Mat4Unit();

		//平移
		template<typename T>
		static Mat4x4<T> Translate(T x, T y, T z);

		template<typename T>
		static Mat4x4<T> Translate(const Vec3<T>& v);
		//旋转
		template<typename T>
		static Mat4x4<T> Rotate(T x, T y, T z);
		template<typename T>
		static Mat4x4<T> Rotate(const Vec3<T>& v);
		//缩放
		template<typename T>
		static Mat4x4<T> Scale(T x, T y, T z);
		template<typename T>
		static Mat4x4<T> Scale(const Vec3<T>& v);

		//view 矩阵
		template<typename T>
		static Mat4x4<T> View(const Vec3<T>& position, const Vec3<T>& front, const Vec3<T>& up);
		//相机矩阵
		template<typename T>
		static Mat4x4<T> LookAt(const Vec3<T>& position, const Vec3<T>& target, const Vec3<T>& up);
		//正交
		template<typename T>
		static Mat4x4<T> Ortho(T left, T right, T bottom, T top, T _near, T _far);
		//透视矩阵-定义平截头体
		template<typename T>
		static Mat4x4<T> Frustum(T left, T right, T bottom, T top, T _near, T _far);
		//透视矩阵
		template<typename T>
		static Mat4x4<T> Projection(T fovy, T aspect, T _near, T _far);
	};

	template<>
	struct alignas(16) Vec4<float>
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};
			struct
			{
				float r;
				float g;
				float b;
				float a;
			};
			__m128 data_m128;
		};

		constexpr Vec4(float v = 0);
		constexpr Vec4(float x, float y, float z, float w = 1.f);
		constexpr Vec4(const Vec2<float>& vec2, float z = 0, float w = 0);
		constexpr Vec4(Vec3<float> vec3, float w = 0);
		constexpr Vec4(const Vec4& vec4) = default;

		constexpr Vec4(__m128 data);
		constexpr operator __m128() const noexcept;

		Vec4 _vectorcall Pow(float rhs) const noexcept;
		Vec4 _vectorcall Sqrt() const noexcept;
		float _vectorcall Dot(Vec4 rhs) const noexcept;

		Vec4 _vectorcall operator-() noexcept;
		Vec4& _vectorcall operator+=(Vec4 rhs) noexcept;
		Vec4& _vectorcall operator+=(float rhs) noexcept;
		Vec4& _vectorcall operator-=(Vec4 rhs) noexcept;
		Vec4& _vectorcall operator-=(float rhs) noexcept;
		Vec4& _vectorcall operator*=(Vec4 rhs) noexcept;
		Vec4& _vectorcall operator*=(float rhs) noexcept;
		Vec4& _vectorcall operator/=(Vec4 rhs) noexcept;
		Vec4& _vectorcall operator/=(float rhs) noexcept;

		friend  Vec4 _vectorcall operator+(Vec4 lhs, Vec4 rhs) noexcept;
		friend  Vec4 _vectorcall operator+(float lhs, Vec4 rhs) noexcept;
		friend  Vec4 _vectorcall operator+(Vec4 lhs, float rhs) noexcept;
		friend  Vec4 _vectorcall operator-(Vec4 lhs, Vec4 rhs) noexcept;
		friend  Vec4 _vectorcall operator-(float lhs, Vec4 rhs) noexcept;
		friend  Vec4 _vectorcall operator-(Vec4 lhs, float rhs) noexcept;
		friend  Vec4 _vectorcall operator*(Vec4 lhs, Vec4 rhs) noexcept;
		friend  Vec4 _vectorcall operator*(float lhs, Vec4 rhs) noexcept;
		friend  Vec4 _vectorcall operator*(Vec4 lhs, float rhs) noexcept;
		friend  Vec4 _vectorcall operator/(Vec4 lhs, Vec4 rhs) noexcept;
		friend  Vec4 _vectorcall operator/(float lhs, Vec4 rhs) noexcept;
		friend  Vec4 _vectorcall operator/(Vec4 lhs, float rhs) noexcept;
	};

	template<>
	struct alignas(16) Quaternions<float>
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};
			__m128 data_m128;
		};

		constexpr Quaternions();
		constexpr Quaternions(Vec4<float>);
		Quaternions(Vec3<float>);
		Quaternions(Vec3<float>, float);
		constexpr Quaternions(float, float, float, float);
		constexpr Quaternions(__m128 data);
		constexpr operator __m128() const noexcept;
		//转换为欧拉角
		Vec3<float> ToEulerAngles() const;

		//转换为旋转矩阵
		Mat4x4<float> ToMat4() const;

		//转换为旋转矩阵
		Mat3x3<float> ToMat3() const;

		//乘四元数
		Quaternions  __vectorcall operator*(Quaternions rhs) const;
		//归一化
		Quaternions __vectorcall Normalize() const;
		//求逆
		Quaternions __vectorcall Inverse() const;
		//乘向量
		Vec4<float> __vectorcall operator*(Vec4<float> rhs) const;
		//线性插值
		Quaternions Lerp(Quaternions rhs, float t) const;
		//球面插值
		Quaternions SLerp(Quaternions rhs, float t) const;
	};

	template<>
	struct alignas(16) Vec3<float>
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float _w;
			};
			struct
			{
				float r;
				float g;
				float b;
				float _a;
			};
			__m128 data_m128;
		};

		static constexpr union {
			struct
			{
				unsigned int x;
				unsigned int y;
				unsigned int z;
				unsigned int w;
			};
			__m128 mask;
		} mask3 = { {  0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000  } };

		constexpr Vec3(float v = 0);
		constexpr Vec3(float x, float y, float z);
		constexpr Vec3(Vec4<float> vec4);
		constexpr Vec3(Vec2<float> vec2, float z = 0);
		constexpr Vec3(const Vec3<float>&) = default;
		constexpr Vec3(__m128 data);
		constexpr operator __m128() const noexcept;

		Vec4<float> ToHomoCoord() const noexcept;
		Vec3 Normalize() const noexcept;
		Vec3 _vectorcall Cross(Vec3 rhs) const noexcept;
		float _vectorcall Dot(Vec3 rhs) const noexcept;
		float _vectorcall Length() const noexcept;
		Vec3 _vectorcall Pow(float rhs) const noexcept;
		Vec3  _vectorcall Sqrt() const noexcept;
		Vec3 _vectorcall Reflect(Vec3 normal) const noexcept;
		Quaternions<float> _vectorcall EularAngleToQuaternions() const noexcept;

		Vec3 _vectorcall operator-() const noexcept;
		Vec3& _vectorcall operator+=(Vec3 rhs) noexcept;
		Vec3& _vectorcall operator+=(float rhs) noexcept;
		Vec3& _vectorcall operator-=(Vec3 rhs) noexcept;
		Vec3& _vectorcall operator-=(float rhs) noexcept;
		Vec3& _vectorcall operator*=(Vec3 rhs) noexcept;
		Vec3& _vectorcall operator*=(float rhs) noexcept;
		Vec3& _vectorcall operator/=(Vec3 rhs) noexcept;
		Vec3& _vectorcall operator/=(float rhs) noexcept;

		friend  Vec3 _vectorcall operator+(Vec3 lhs, Vec3 rhs) noexcept;
		friend  Vec3 _vectorcall operator+(float lhs, Vec3 rhs) noexcept;
		friend  Vec3 _vectorcall operator+(Vec3 lhs, float rhs) noexcept;
		friend  Vec3 _vectorcall operator-(Vec3 lhs, Vec3 rhs) noexcept;
		friend  Vec3 _vectorcall operator-(float lhs, Vec3 rhs) noexcept;
		friend  Vec3 _vectorcall operator-(Vec3 lhs, float rhs) noexcept;
		friend  Vec3 _vectorcall operator*(Vec3 lhs, Vec3 rhs) noexcept;
		friend  Vec3 _vectorcall operator*(float lhs, Vec3 rhs) noexcept;
		friend  Vec3 _vectorcall operator*(Vec3 lhs, float rhs) noexcept;
		friend  Vec3 _vectorcall operator/(Vec3 lhs, Vec3 rhs) noexcept;
		friend  Vec3 _vectorcall operator/(float lhs, Vec3 rhs) noexcept;
		friend  Vec3 _vectorcall operator/(Vec3 lhs, float rhs) noexcept;
	};

	template<>
	struct alignas(16) Mat4x4<float>
	{
		union
		{
			float data[16];
			__m128 column[4];
		};
		//修改为列矩阵
		constexpr Mat4x4(float _0, float _1, float _2, float _3, float _4, float _5, float _6, float _7, float _8, float _9, float _10, float _11, float _12, float _13, float _14, float _15);

		constexpr Mat4x4(__m128 c1, __m128 c2, __m128 c3, __m128 c4);

		constexpr Mat4x4(float a, float b, float c, float d);

		constexpr Mat4x4(Vec4<float> diag);

		constexpr Mat4x4();

		//矩阵乘法
		Mat4x4<float> operator*(const Mat4x4<float>& rhs) const;

		//乘向量 4分量
		Vec4<float> _vectorcall operator*(Vec4<float> rhs) const;

		//转置
		Mat4x4<float> Transpose() const;

		//转换到3x3矩阵
		Mat3x3<float> ToMat3x3() const;
	};

	template<>
	struct alignas(16) Mat3x3<float>
	{
		union
		{
			float data[4 * 3];
			__m128 column[3];
		};

		constexpr Mat3x3();
		constexpr Mat3x3(__m128, __m128, __m128);
		constexpr Mat3x3(float a, float b, float c, float d, float e, float f, float g, float h, float i);

		//乘矩阵
		Mat3x3<float> operator*(const Mat3x3& rhs) const;
		//乘向量
		Vec3<float> _vectorcall operator*(Vec3<float> rhs) const;
		//转置
		Mat3x3<float> Transpose() const;
		//求逆
		Mat3x3<float> Inverse() const;
		//转换到4x4矩阵
		Mat4x4<float> ToMat4x4() const;
	};
};

#include "gmath.inl"
