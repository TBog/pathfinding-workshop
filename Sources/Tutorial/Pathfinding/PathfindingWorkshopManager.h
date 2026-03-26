#pragma once

#ifndef __PATHFINDINGWORKSHOPMANAGER_H__
#define __PATHFINDINGWORKSHOPMANAGER_H__

#include "..\Utils\DynVec.h"
#include "Types.h"

namespace Pathfinding
{
	class PathfindingWorkSheet;
	struct TriangulationConstraint;
}

//===================================================================
//	CLASS PathfindingWorkshopManager
//===================================================================

class PathfindingWorkshopManager
{
private:
	//---------------------------------------------------------------
	//	CONSTRUCTOR / DESTRUCTOR
	//---------------------------------------------------------------
	PathfindingWorkshopManager();
	virtual ~PathfindingWorkshopManager();

public:
	//---------------------------------------------------------------
	//	SINGLETON FUNCTIONS
	//---------------------------------------------------------------
	static PathfindingWorkshopManager* Get() { return s_Instance; }

	static void Create();
	static void Destroy();

	//---------------------------------------------------------------
	//	MAIN FUNCTIONS
	//---------------------------------------------------------------
	void Update(float dt);

protected:

	void _DrawDebugMenu();
	void _RunSignedAreaExercise();
	void _RunInsideTriangleExercise();
	void _RunInsideTriangleCircumcircleExercise();
	void _RunConvexHullExercise();
	void _RunRandomTriangulationExercise();
	void _RunRandomTriangulationExercise2();
	void _RunDelaunayTriangulationExercise();
	void _RunConstrainedDelaunayExercise();
	void _RunGridPathfindingExercise();
	void _RunAStarPathfindingExercise();
	void _RunAStarPathfindingSmoothExercise();

	static PathfindingWorkshopManager* s_Instance;
	enum class Exercise
	{
		SignedArea,
		InsideTriangle,
		InsideTriangleCircumcircle,
		ConvexHull,
		RandomTriangulation,
		DelaunayTriangulation,
		ConstrainedDelaunay,
		GridPathfinding,
		AStarPathfinding,
		AStarPathfindingSmooth,
		_Count
	};

	using ExerciseFunc = void (PathfindingWorkshopManager::*)();

	struct MenuItem
	{
		const wchar_t* name;
		const int subItemCount;
		int subItem{ 0 };
		ExerciseFunc func;

		MenuItem(const wchar_t* name, int subItemCount, ExerciseFunc func) : name(name), subItemCount(subItemCount), func(func) {};
	};

	MenuItem MENU_ITEMS_EXERCISES[static_cast<int>(Exercise::_Count)] = {
		{L"Signed Area", 0, &PathfindingWorkshopManager::_RunSignedAreaExercise},
		{L"Inside Triangle", 0, &PathfindingWorkshopManager::_RunInsideTriangleExercise},
		{L"Inside Triangle Circumcircle", 0, &PathfindingWorkshopManager::_RunInsideTriangleCircumcircleExercise},
		{L"Convex Hull", 0, &PathfindingWorkshopManager::_RunConvexHullExercise},
		{L"Random Triangulation", 0, &PathfindingWorkshopManager::_RunRandomTriangulationExercise},
		{L"Delaunay Triangulation", 2, &PathfindingWorkshopManager::_RunDelaunayTriangulationExercise},
		{L"Constrained Delaunay", 2, &PathfindingWorkshopManager::_RunConstrainedDelaunayExercise},
		{L"Grid Pathfinding", 2, &PathfindingWorkshopManager::_RunGridPathfindingExercise},
		{L"A* Pathfinding", 2, &PathfindingWorkshopManager::_RunAStarPathfindingExercise},
		{L"A* Pathfinding Smooth", 2, &PathfindingWorkshopManager::_RunAStarPathfindingSmoothExercise},
	};

	void GenerateRandomPointsAndObstacles(int pointCount, DynVec<Pathfinding::Vector2>& obstacleCenters, DynVec<Pathfinding::PointId>& obstaclesIndexes, DynVec<Pathfinding::TriangulationConstraint>& constraints);
	bool IsInsideObstacle(DynVec<Pathfinding::Vector2>& obstacleCenters, DynVec<Pathfinding::PointId>& obstaclesIndexes, const Pathfinding::Vector2& point) const;

	unsigned int m_randomSeed{ 1337 };
	float m_rotatingAngle{ 0.f };
	bool m_rotationEnabled{ true };

	bool m_upPressed{ false };
	bool m_downPressed{ false };
	bool m_leftPressed{ false };
	bool m_rightPressed{ false };
	bool m_acceptPressed{ false };
	bool m_showDebugMenu{ false };
	bool m_pausePressed{ false };
	int m_menuItem{ 0 };
	int GetSelectedMenuSubItem() const;
	Exercise m_selectedExercise;
	Pathfinding::PathfindingWorkSheet* m_userWorkSheet{ nullptr };
	Pathfinding::PathfindingWorkSheet* m_controlWorkSheet{ nullptr };
	DynVec<Pathfinding::Vector2> m_points;

	const float PI = (float)D3DX_PI;
	const int OBSTACLE_POINT_COUNT = 5;
};

#define g_pathfindingWorkshopManager PathfindingWorkshopManager::Get()

#endif // __PATHFINDINGWORKSHOPMANAGER_H__
