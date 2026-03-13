#pragma once

#include "PathfindingWorkSheet.h"

namespace Pathfinding
{

	class ControlPathfindingWorkSheet : public PathfindingWorkSheet
	{
	public:
		//---------------------------------------------------------------
		// Solution
		//---------------------------------------------------------------

		// Line Side Exercise   
		float SignedArea(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.x * p3.y - p3.x * p2.y - p2.x * p1.y;
		}

		// Line Side Exercise
		bool IsLeft(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return SignedArea(p1, p2, p3) > FLT_EPSILON;
		}

		bool IsCollinear(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return fabsf(SignedArea(p1, p2, p3)) <= FLT_EPSILON;
		}

		bool InsideTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) override
		{
			const float s1 = SignedArea(p1, p2, p4);
			const float s2 = SignedArea(p2, p3, p4);
			const float s3 = SignedArea(p3, p1, p4);

			return (s1 <= 0 && s2 <= 0 && s3 <= 0) || (s1 >= 0 && s2 >= 0 && s3 >= 0);
			//return IsLeft(p1, p2, p4) && IsLeft(p2, p3, p4) && IsLeft(p3, p1, p4);
		}

		bool InsideCircumcircle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) override
		{
			const float ax = p1.x - p4.x;
			const float bx = p2.x - p4.x;
			const float cx = p3.x - p4.x;
			const float ay = p1.y - p4.y;
			const float by = p2.y - p4.y;
			const float cy = p3.y - p4.y;

			const float val = ax * by * (cx * cx + cy * cy) + bx * cy * (ax * ax + ay * ay) + cx * ay * (bx * bx + by * by) -
				ax * cy * (bx * bx + by * by) - bx * ay * (cx * cx + cy * cy) - cx * by * (ax * ax + ay * ay);

			if (IsLeft(p1, p2, p3))
			{
				return val > 0;
			}
			else
			{
				return val < 0;
			}
		}

		void ConvexHull(const DynVec<Vector2>& points, DynVec<int>& outHull)
		{
		}

		void RandomTriangulation(const DynVec<Vector2>& points, Triangulation& triangulation)
		{
		}

		void EdgeFlipping(const Triangulation& inTriangulation, Triangulation& outTriangulation)
		{
		}

		void AddTriangulationConstraints(const Triangulation& inTriangulation, const TriangulationConstraint& constraint, Triangulation& outTriangulation)
		{
		}

		void GridPathfinding(const DynVec<DynVec<int>>& map, const Cell& start, const Cell& goal, DynVec<Cell>& outPath)
		{
		}

		void AStarPathfinding(const Triangulation& triangulation, const Vector2& startPoint, const Vector2& goalPoint, DynVec<Vector2>& outPath)
		{
		}

		void SmoothPath(const Triangulation& triangulation, const DynVec<Vector2>& path, DynVec<Vector2>& outPath)
		{
		}
	};

} //namespace Pathfinding