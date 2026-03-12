#pragma once

#ifndef __PATHFINDINGWORKSHOPMANAGER_H__
#define __PATHFINDINGWORKSHOPMANAGER_H__


class PathfindingWorkSheet;

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
	void Render();

protected:

	void _RunSignedAreaExercise();

	static PathfindingWorkshopManager* s_Instance;

	PathfindingWorkSheet* m_UserWorkSheet{ nullptr };
	PathfindingWorkSheet* m_ControlWorkSheet{ nullptr };
};

#define g_pathfindingWorkshopManager PathfindingWorkshopManager::Get()

#endif // __PATHFINDINGWORKSHOPMANAGER_H__
