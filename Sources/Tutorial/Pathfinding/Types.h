#pragma once

#include <limits>

namespace Pathfinding
{
	typedef D3DXVECTOR2 Vector2;
	typedef D3DXVECTOR3 Vector3;
	typedef D3DXVECTOR4 Vector4;
	typedef D3DXMATRIX Mat4x4;
	typedef D3DXCOLOR Color;

	// Common color definitions
	inline const Color COLOR_WHITE = Color(1.0f, 1.0f, 1.0f, 1.0f);
	inline const Color COLOR_BLACK = Color(0.0f, 0.0f, 0.0f, 1.0f);
	inline const Color COLOR_RED = Color(1.0f, 0.0f, 0.0f, 1.0f);
	inline const Color COLOR_GREEN = Color(0.0f, 1.0f, 0.0f, 1.0f);
	inline const Color COLOR_BLUE = Color(0.0f, 0.0f, 1.0f, 1.0f);
	inline const Color COLOR_YELLOW = Color(1.0f, 1.0f, 0.0f, 1.0f);
	inline const Color COLOR_CYAN = Color(0.0f, 1.0f, 1.0f, 1.0f);
	inline const Color COLOR_MAGENTA = Color(1.0f, 0.0f, 1.0f, 1.0f);
	inline const Color COLOR_GRAY = Color(0.5f, 0.5f, 0.5f, 1.0f);

	inline Color WithAlpha(const Color& base, float alpha)
	{
		return Color(base.r, base.g, base.b, alpha);
	}

	inline const Vector2 VECTOR2_NAN = Vector2(std::numeric_limits<float>::quiet_NaN(), std::numeric_limits<float>::quiet_NaN());
}