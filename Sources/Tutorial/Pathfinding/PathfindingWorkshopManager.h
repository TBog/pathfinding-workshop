#pragma once

#ifndef __PATHFINDINGWORKSHOPMANAGER_H__
#define __PATHFINDINGWORKSHOPMANAGER_H__

#include "..\Utils\DynVec.h"

class Entity;
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

	static void                     Create();
	static void                     Destroy();

	//---------------------------------------------------------------
	//	MAIN FUNCTIONS
	//---------------------------------------------------------------
	void                            DeleteAllEntities();
	void                            Update(float dt);
	void                            Render();

	const DynVec<Entity*>& GetEntitiesList() const { return m_entitiesList; }

protected:
	static PathfindingWorkshopManager* s_Instance;

	DynVec<Entity*>                 m_entitiesList;

	PathfindingWorkSheet* m_UserWorkSheet{ nullptr };
	PathfindingWorkSheet* m_ControlWorkSheet{ nullptr };
};

#define g_pathfindingWorkshopManager           PathfindingWorkshopManager::Get()

#endif // __PATHFINDINGWORKSHOPMANAGER_H__
