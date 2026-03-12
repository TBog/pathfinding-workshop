#include "pch.h"

#include "PathfindingWorkshopManager.h"
#include "UserPathfindingWorkSheet.h"
#include "ControlPathfindingWorkSheet.h"

#include "..\Utils\Entity.h"
#include "..\Utils\Utils.h"

//-------------------------------------------------------------------

PathfindingWorkshopManager* PathfindingWorkshopManager::s_Instance = NULL;

PathfindingWorkshopManager::PathfindingWorkshopManager()
	: m_UserWorkSheet(new UserPathfindingWorkSheet())
	, m_ControlWorkSheet(new ControlPathfindingWorkSheet())
{
}

//-------------------------------------------------------------------

PathfindingWorkshopManager::~PathfindingWorkshopManager()
{
	SAFE_DELETE(m_UserWorkSheet);
	SAFE_DELETE(m_ControlWorkSheet);
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

void PathfindingWorkshopManager::Update(float dt)
{
	_RunSignedAreaExercise();

	m_UserWorkSheet->Update(dt);
	m_ControlWorkSheet->Update(dt);
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::Render()
{
	m_UserWorkSheet->Render();
	m_ControlWorkSheet->Render();
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::_RunSignedAreaExercise()
{
	const float scale = 3.f;
	const int pointsCount = 16;
	Vector3 points[pointsCount];
	for (int i = 0; i < pointsCount; i++)
	{
		float angle = (float)i / (float)pointsCount * 2.f * (float)D3DX_PI;
		points[i] = Vector3(cosf(angle) * scale, .5f, sinf(angle) * scale);
	}
	for (int i = 1; i < pointsCount; i++)
	{
		Vector3 p1{0.f, 1.f, 0.f};
		Vector3 p2 = points[(i - 1) % pointsCount];
		Vector3 p3 = points[(i - 0) % pointsCount];

		g_debugRender->AddLine(p1, p2, Color(1.f, 1.f, 1.f, 1.f));
		
		Vector2 testPoints[] = { Vector2(p1.x, p1.z), Vector2(p2.x, p2.z), Vector2(p3.x, p3.z) };
		const bool matchingResults = m_UserWorkSheet->IsLeft(testPoints[0], testPoints[1], testPoints[2]) == m_ControlWorkSheet->IsLeft(testPoints[0], testPoints[1], testPoints[2]);

		// use debug render to draw a red of green check for each test
		g_debugRender->AddTriangle(p1, p2, p3, matchingResults ? Color(0.f, 1.f, 1.f, .5f) : Color(1.f, 0.f, .5f, .5f));
	}
}