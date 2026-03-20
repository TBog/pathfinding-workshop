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
	void _RunDelaunayTriangulationExercise();
	void _RunConstrainedDelaunayExercise();
	void _RunGridPathfindingExercise();
	void _RunAStarPathfindingExercise();

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
		_Count
	};
	using ExerciseFunc = void (PathfindingWorkshopManager::*)();
	static constexpr ExerciseFunc EXERCISE_FUNCS[] = {
		&PathfindingWorkshopManager::_RunSignedAreaExercise,
		&PathfindingWorkshopManager::_RunInsideTriangleExercise,
		&PathfindingWorkshopManager::_RunInsideTriangleCircumcircleExercise,
		&PathfindingWorkshopManager::_RunConvexHullExercise,
		&PathfindingWorkshopManager::_RunRandomTriangulationExercise,
		&PathfindingWorkshopManager::_RunDelaunayTriangulationExercise,
		&PathfindingWorkshopManager::_RunConstrainedDelaunayExercise,
		&PathfindingWorkshopManager::_RunGridPathfindingExercise,
		&PathfindingWorkshopManager::_RunAStarPathfindingExercise
	};
	const wchar_t* MENU_ITEMS_EXERCISES[static_cast<int>(Exercise::_Count)] = {
		L"SignedArea",
		L"InsideTriangle",
		L"InsideTriangleCircumcircle",
		L"ConvexHull",
		L"RandomTriangulation",
		L"DelaunayTriangulation",
		L"ConstrainedDelaunay",
		L"GridPathfinding",
		L"AStarPathfinding",
	};

	void GenerateRandomPointsAndObstacles(int pointCount, DynVec<Pathfinding::Vector2>& obstacleCenters, DynVec<Pathfinding::PointId>& obstaclesIndexes, DynVec<Pathfinding::TriangulationConstraint>& constraints);
	bool IsInsideObstacle(DynVec<Pathfinding::Vector2>& obstacleCenters, DynVec<Pathfinding::PointId>& obstaclesIndexes, const Pathfinding::Vector2& point) const;

	unsigned int m_randomSeed{ 1337 };
	float m_rotatingAngle{ 0.f };
	bool m_upPressed{ false };
	bool m_downPressed{ false };
	bool m_leftPressed{ false };
	bool m_rightPressed{ false };
	bool m_acceptPressed{ false };
	bool m_showDebugMenu{ false };
	int m_menuItem{ 0 };
	Exercise m_selectedExercise;
	Pathfinding::PathfindingWorkSheet* m_userWorkSheet{ nullptr };
	Pathfinding::PathfindingWorkSheet* m_controlWorkSheet{ nullptr };
	DynVec<Pathfinding::Vector2> m_points;

	const float PI = (float)D3DX_PI;
	const int OBSTACLE_POINT_COUNT = 5;
};

#define g_pathfindingWorkshopManager PathfindingWorkshopManager::Get()

#endif // __PATHFINDINGWORKSHOPMANAGER_H__
