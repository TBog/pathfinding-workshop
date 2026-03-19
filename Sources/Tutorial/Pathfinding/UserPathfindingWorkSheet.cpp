#include "pch.h"

#include "UserPathfindingWorkSheet.h"
#include "Utils.h"

float Pathfinding::UserPathfindingWorkSheet::SignedArea(const Vector2& p1, const Vector2& p2, const Vector2& p3)
{
	return false;
}

bool Pathfinding::UserPathfindingWorkSheet::IsLeft(const Vector2& p1, const Vector2& p2, const Vector2& p3)
{
	return false;
}

bool Pathfinding::UserPathfindingWorkSheet::IsCollinear(const Vector2& p1, const Vector2& p2, const Vector2& p3)
{
	return false;
}

bool Pathfinding::UserPathfindingWorkSheet::InsideTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4)
{
	return false;
}

bool Pathfinding::UserPathfindingWorkSheet::InsideCircumcircle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4)
{
	return false;
}

void Pathfinding::UserPathfindingWorkSheet::ConvexHull(const DynVec<Vector2>& points, DynVec<PointId>& outHull)
{
	if (points.GetSize() > 2)
	{
		outHull.Add(PointId(0));
		outHull.Add(PointId(1));
		outHull.Add(PointId(2));
	}
}

void Pathfinding::UserPathfindingWorkSheet::RandomTriangulation(const DynVec<Vector2>& points, Triangulation& triangulation)
{

}

void Pathfinding::UserPathfindingWorkSheet::EdgeFlipping(Triangulation& triangulation, size_t maxIterations)
{

}

void Pathfinding::UserPathfindingWorkSheet::AddTriangulationConstraint(Triangulation& triangulation, const TriangulationConstraint& constraint)
{

}

void Pathfinding::UserPathfindingWorkSheet::GridPathfinding(const Grid2D<int>& map, const Cell& start, const Cell& goal, DynVec<Cell>& outPath)
{

}

void Pathfinding::UserPathfindingWorkSheet::AStarPathfinding(const Triangulation& triangulation, const Vector2& startPoint, const Vector2& goalPoint, DynVec<Vector2>& outPath)
{

}

void Pathfinding::UserPathfindingWorkSheet::SmoothPath(const Triangulation& triangulation, const DynVec<Vector2>& path, DynVec<Vector2>& outPath)
{

}
