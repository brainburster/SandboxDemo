#pragma once

#include "gmath.hpp"

using iVec2 = gmath::Vec2<int>;
using Vec2 = gmath::Vec2<float>;
using Vec3 = gmath::Vec3<float>;
using Vec4 = gmath::Vec4<float>;
using Mat3 = gmath::Mat3x3<float>;
using Mat4 = gmath::Mat4x4<float>;
using Color4f = Vec4;
using gmath::utility::Clamp;

//为了兼容c++20之前的版本
#define U8(x)  ((const char*)u8##x)
//其实应该是ABGR,因为是小端存储
#define RGBA(r,g,b,a) ((unsigned int)(((unsigned int)(r)|((unsigned int)(g)<<8))|(((unsigned int)(b)<<16))|(((unsigned int)(a))<<24)))

constexpr unsigned int RGBA2ABGR(unsigned int color)
{
	return ((color >> 24) | ((color & 0x00FF0000) >> 8) | ((color & 0x0000FF00) << 8) | ((color & 0x000000FF) << 24));
}

inline unsigned int Color4f2ColorUInt32(Color4f color)
{
	return RGBA(
		Clamp((unsigned int)(color.r * 255), 0u, 255u),
		Clamp((unsigned int)(color.g * 255), 0u, 255u),
		Clamp((unsigned int)(color.b * 255), 0u, 255u),
		Clamp((unsigned int)(color.a * 255), 0u, 255u)
	);
}

union bit8tool
{
	struct
	{
		bool _0 : 1;
		bool _1 : 1;
		bool _2 : 1;
		bool _3 : 1;
		bool _4 : 1;
		bool _5 : 1;
		bool _6 : 1;
		bool _7 : 1;
	};
	uint8_t data;
	bit8tool(bool __0, bool __1, bool __2, bool __3, bool __4, bool __5, bool __6, bool __7) :
		_0(__7), _1(__6), _2(__5), _3(__4), _4(__3), _5(__2), _6(__1), _7(__0)
	{}
};
