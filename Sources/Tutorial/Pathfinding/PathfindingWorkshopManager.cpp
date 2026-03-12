#include "pch.h"

#include "PathfindingWorkshopManager.h"
#include "UserPathfindingWorkSheet.h"
#include "ControlPathfindingWorkSheet.h"

#include "..\Utils\Entity.h"
#include "..\Utils\Utils.h"

//-------------------------------------------------------------------

PathfindingWorkshopManager* PathfindingWorkshopManager::s_Instance = NULL;

PathfindingWorkshopManager::PathfindingWorkshopManager()
	: m_entitiesList(1024, 1024)
	, m_UserWorkSheet(new UserPathfindingWorkSheet())
	, m_ControlWorkSheet(new ControlPathfindingWorkSheet())
{
}

//-------------------------------------------------------------------

PathfindingWorkshopManager::~PathfindingWorkshopManager()
{
	SAFE_DELETE(m_UserWorkSheet);
	SAFE_DELETE(m_ControlWorkSheet);
	DeleteAllEntities();
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::Create()
{
	if (s_Instance)
	{
		myAssert(false, L"PathfindingWorkshopManager::Create() already called !");
		return;
	}

	s_Instance = new PathfindingWorkshopManager();
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::Destroy()
{
	SAFE_DELETE(s_Instance);
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::DeleteAllEntities()
{
	for (int i = 0; i < m_entitiesList.GetSize(); i++)
	{
		SAFE_RELEASE(m_entitiesList[i]);
	}

	m_entitiesList.Clear();
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::Update(float dt)
{
	m_UserWorkSheet->Update(dt);
	m_ControlWorkSheet->Update(dt);
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::Render()
{
	m_UserWorkSheet->Render();
	m_ControlWorkSheet->Render();
}
