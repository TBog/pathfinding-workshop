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

	void _RunSignedAreaExercise();

	static PathfindingWorkshopManager* s_Instance;

	float m_RotatingAngle{ 0.f };
	Pathfinding::PathfindingWorkSheet* m_UserWorkSheet{ nullptr };
	Pathfinding::PathfindingWorkSheet* m_ControlWorkSheet{ nullptr };
};

#define g_pathfindingWorkshopManager PathfindingWorkshopManager::Get()

#endif // __PATHFINDINGWORKSHOPMANAGER_H__
