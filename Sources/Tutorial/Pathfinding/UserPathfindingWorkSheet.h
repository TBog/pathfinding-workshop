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

		float SignedArea(const Vector2& p1, const Vector2& p2, const Vector2& p3) override;
		bool IsLeft(const Vector2& p1, const Vector2& p2, const Vector2& p3) override;
		bool IsCollinear(const Vector2& p1, const Vector2& p2, const Vector2& p3) override;
		bool InsideTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) override;
		bool InsideCircumcircle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) override;
		void ConvexHull(const DynVec<Vector2>& points, DynVec<PointId>& outHull) override;
		void RandomTriangulation(const DynVec<Vector2>& points, Triangulation& triangulation) override;
		void EdgeFlipping(Triangulation& triangulation, size_t maxIterations) override;
		void AddTriangulationConstraint(Triangulation& triangulation, const TriangulationConstraint& constraint) override;
		void GridPathfinding(const Grid2D<int>& map, const Cell& start, const Cell& goal, DynVec<Cell>& outPath) override;
		void AStarPathfinding(const Triangulation& triangulation, const Vector2& startPoint, const Vector2& goalPoint, DynVec<Vector2>& outPath) override;
		void SmoothPath(const Triangulation& triangulation, const DynVec<Vector2>& path, DynVec<Vector2>& outPath) override;
	};

} // namespace Pathfinding