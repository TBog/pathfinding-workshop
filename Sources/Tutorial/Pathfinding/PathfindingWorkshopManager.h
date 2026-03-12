#pragma once

#ifndef __PATHFINDINGWORKSHOPMANAGER_H__
#define __PATHFINDINGWORKSHOPMANAGER_H__

/// @file PathfindingWorkshopManager.h
/// @brief Singleton manager that drives both pathfinding worksheet instances.

#include "..\Utils\DynVec.h"

class Entity;
class PathfindingWorkSheet;

//===================================================================
//	CLASS PathfindingWorkshopManager
//===================================================================

/// @class PathfindingWorkshopManager
/// @brief Singleton that owns and drives both pathfinding worksheet instances.
///
/// Holds the user's worksheet (@ref UserPathfindingWorkSheet) and the
/// reference solution worksheet (@ref ControlPathfindingWorkSheet).  Both are
/// ticked and rendered every frame.
///
/// Also maintains an independent entity list for workshop-specific scene
/// objects (distinct from the main world entities).
///
/// Access the singleton via the @c g_pathfindingWorkshopManager macro.
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

	/// @brief Returns the singleton instance (may be @c nullptr before Create()).
	static PathfindingWorkshopManager* Get() { return s_Instance; }

	/// @brief Creates the singleton instance.  Must be called exactly once.
	static void                     Create();

	/// @brief Destroys the singleton instance and releases all resources.
	static void                     Destroy();

	//---------------------------------------------------------------
	//	MAIN FUNCTIONS
	//---------------------------------------------------------------

	/// @brief Releases and removes all workshop-specific entities from the list.
	void                            DeleteAllEntities();

	/// @brief Ticks both the user and control worksheets.
	/// @param dt Elapsed time in seconds since the last frame.
	void                            Update(float dt);

	/// @brief Renders both the user and control worksheets.
	void                            Render();

	/// @brief Returns the list of workshop-specific scene entities.
	const DynVec<Entity*>& GetEntitiesList() const { return m_entitiesList; }

protected:
	static PathfindingWorkshopManager* s_Instance;

	/// @brief Workshop-specific scene entities (separate from world entities).
	DynVec<Entity*>                 m_entitiesList;

	/// @brief Student-facing exercise implementation.
	PathfindingWorkSheet* m_UserWorkSheet{ nullptr };

	/// @brief Reference (correct) exercise implementation.
	PathfindingWorkSheet* m_ControlWorkSheet{ nullptr };
};

/// @brief Global convenience accessor for @ref PathfindingWorkshopManager.
#define g_pathfindingWorkshopManager           PathfindingWorkshopManager::Get()

#endif // __PATHFINDINGWORKSHOPMANAGER_H__
