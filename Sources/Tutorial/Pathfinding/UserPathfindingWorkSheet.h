#pragma once

#include "PathfindingWorkSheet.h"

namespace Pathfinding
{

	class UserPathfindingWorkSheet : public PathfindingWorkSheet
	{
	public:
		//---------------------------------------------------------------
		// Exercises
		//---------------------------------------------------------------

		// Line Side Exercise   
		float SignedArea(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return false;
		}

		// Line Side Exercise
		bool IsLeft(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return false;
		}

		bool IsCollinear(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return false;
		}

		bool InsideTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) override
		{
			return false;
		}

		bool InsideCircumcircle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) override
		{
			return false;
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

		void GridPathfinding(const Grid2D<int>& map, const Cell& start, const Cell& goal, DynVec<Cell>& outPath)
		{
		}

		void AStarPathfinding(const Triangulation& triangulation, const Vector2& startPoint, const Vector2& goalPoint, DynVec<Vector2>& outPath)
		{
		}

		void SmoothPath(const Triangulation& triangulation, const DynVec<Vector2>& path, DynVec<Vector2>& outPath)
		{
		}
	};

} // namespace Pathfinding