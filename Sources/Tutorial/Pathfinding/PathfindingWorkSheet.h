#pragma once

#include "..\Render\DebugRender.h"
#include "Types.h"
#include "Triangulation.h"

namespace Pathfinding
{

	struct TriangulationConstraint
	{
		int p1;
		int p2;
	};

	struct Cell
	{
		int x;
		int y;

		Cell(int x, int y) : x(x), y(y) {}
	};

	class PathfindingWorkSheet
	{
	public:
		//---------------------------------------------------------------
		//	CONSTRUCTOR / DESTRUCTOR
		//---------------------------------------------------------------
		PathfindingWorkSheet() {}
		virtual ~PathfindingWorkSheet() {}

		//---------------------------------------------------------------
		//	MAIN FUNCTIONS
		//---------------------------------------------------------------
		void Update(float dt) {}

		//---------------------------------------------------------------
		// Navmesh Exercises
		//---------------------------------------------------------------

		virtual float SignedArea(const Vector2& p1, const Vector2& p2, const Vector2& p3) = 0;
		virtual bool IsLeft(const Vector2& p1, const Vector2& p2, const Vector2& p3) = 0;
		virtual bool IsCollinear(const Vector2& p1, const Vector2& p2, const Vector2& p3) = 0;
		virtual bool InsideTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) = 0;
		virtual bool InsideCircumcircle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) = 0;

		virtual void ConvexHull(const DynVec<Vector2>& points, DynVec<int>& outHull) = 0;
		virtual void RandomTriangulation(const DynVec<Vector2>& points, Triangulation& triangulation) = 0;
		virtual void EdgeFlipping(Triangulation& triangulation, size_t maxIterations = 0) = 0;
		virtual void AddTriangulationConstraints(const Triangulation& inTriangulation, const TriangulationConstraint& constraint, Triangulation& outTriangulation) = 0;

		//---------------------------------------------------------------
		// Pathfinding Exercises
		//---------------------------------------------------------------

		virtual void GridPathfinding(const Grid2D<int>& map, const Cell& start, const Cell& goal, DynVec<Cell>& outPath) = 0;
		virtual void AStarPathfinding(const Triangulation& triangulation, const Vector2& startPoint, const Vector2& goalPoint, DynVec<Vector2>& outPath) = 0;
		virtual void SmoothPath(const Triangulation& triangulation, const DynVec<Vector2>& path, DynVec<Vector2>& outPath) = 0;
	};
}