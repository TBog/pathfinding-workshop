#pragma once

#include "Types.h"

namespace Pathfinding
{
	template<typename T>
	inline void Swap(T& lhs, T& rhs)
	{
		T temp = lhs;
		lhs = rhs;
		rhs = temp;
	}

	inline void Normalize(Vector2& v)
	{
		D3DXVec2Normalize(&v, &v);
	}

	inline void Normalize(Vector3& v)
	{
		D3DXVec3Normalize(&v, &v);
	}

	inline void Normalize(Vector4& v)
	{
		D3DXVec4Normalize(&v, &v);
	}

	inline Vector2 GetNormalized(const Vector2& v)
	{
		Vector2 result;
		D3DXVec2Normalize(&result, &v);
		return result;
	}

	inline Vector3 GetNormalized(const Vector3& v)
	{
		Vector3 result;
		D3DXVec3Normalize(&result, &v);
		return result;
	}

	inline Vector4 GetNormalized(const Vector4& v)
	{
		Vector4 result;
		D3DXVec4Normalize(&result, &v);
		return result;
	}

	template <typename T>
	DynVec<T> DynVecFromStdVector(const std::vector<T>& vec, int grow = -1)
	{
		DynVec<T> result(static_cast<int>(vec.size()), grow < 0 ? vec.size() : grow);
		for (const auto& item : vec)
			result.Add(item);
		return result;
	}

	inline bool FindIntersection(const Vector2& x1, const Vector2& y1, const Vector2& x2, const Vector2& y2, Vector2& intersection)
	{
		float a1 = y1.y - x1.y;
		float b1 = x1.x - y1.x;
		float c1 = a1 * x1.x + b1 * x1.y;

		float a2 = y2.y - x2.y;
		float b2 = x2.x - y2.x;
		float c2 = a2 * x2.x + b2 * x2.y;

		float delta = a1 * b2 - a2 * b1;

		//If lines are parallel, the result will be (NaN, NaN).        
		if (delta == 0)
		{
			intersection = VECTOR2_NAN;
			return false;
		}
		else
		{
			intersection = Vector2((b2 * c1 - b1 * c2) / delta, (a1 * c2 - a2 * c1) / delta);
			return true;
		}
	}

	inline float DistSqr(const Vector2& x, const Vector2& y)
	{
		const Vector2 d = x - y;
		return d.x * d.x + d.y * d.y;
	}

	inline float Distance(const Vector2& x, const Vector2& y)
	{
		return sqrtf(DistSqr(x, y));
	}

	inline bool FindSegmentIntersection(const Vector2& x1, const Vector2& y1, const Vector2& x2, const Vector2& y2, Vector2& inter)
	{
		Vector2 lineInter;
		if (!FindIntersection(x1, y1, x2, y2, lineInter))
		{
			inter = VECTOR2_NAN;
			return false;
		}

		// Check the intersection point is part of the two segments
		if (DistSqr(x1, lineInter) < DistSqr(x1, y1) && DistSqr(y1, lineInter) < DistSqr(x1, y1) &&
			DistSqr(x2, lineInter) < DistSqr(x2, y2) && DistSqr(y2, lineInter) < DistSqr(x2, y2))
		{
			inter = lineInter;
			return true;
		}

		inter = VECTOR2_NAN;
		return false;
	}

}