#pragma once

#ifndef __PATHFINDINGWORKSHOPMANAGER_H__
#define __PATHFINDINGWORKSHOPMANAGER_H__

namespace Pathfinding
{
	class PathfindingWorkSheet;
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


	float m_rotatingAngle{ 0.f };
	bool m_upPressed{ false };
	bool m_downPressed{ false };
	bool m_acceptPressed{ false };
	bool m_showDebugMenu{ false };
	int m_menuItem{ 0 };
	Exercise m_selectedExercise;
	Pathfinding::PathfindingWorkSheet* m_userWorkSheet{ nullptr };
	Pathfinding::PathfindingWorkSheet* m_controlWorkSheet{ nullptr };
};

#define g_pathfindingWorkshopManager PathfindingWorkshopManager::Get()

#endif // __PATHFINDINGWORKSHOPMANAGER_H__
