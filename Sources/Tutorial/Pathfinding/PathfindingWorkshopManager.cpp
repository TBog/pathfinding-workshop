#include "pch.h"

#include "PathfindingWorkshopManager.h"
#include "UserPathfindingWorkSheet.h"
#include "ControlPathfindingWorkSheet.h"

#include "..\Utils\Entity.h"
#include "..\Utils\Utils.h"

using namespace Pathfinding;
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

	m_RotatingAngle += dt * .5f;
	if (m_RotatingAngle > 360.f)
	{
		m_RotatingAngle -= 360.f;
	}
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::_RunSignedAreaExercise()
{
	const int pointsCount = 8;
	Vector3 points[pointsCount];
	for (int i = 0; i < pointsCount; i++)
	{
		float angle = (float)i / (float)pointsCount * 2.f * (float)D3DX_PI;
		points[i] = Vector3(cosf(angle), 0.f, sinf(angle)) * 3.f;
	}

	const Vector3 lineA(cosf(m_RotatingAngle) * 5.f, 0.f, sinf(m_RotatingAngle) * 5.f);
	const Vector3 lineB(cosf(m_RotatingAngle) * -5.f, 0.f, sinf(m_RotatingAngle) * -5.f);

	for (int i = 0; i < pointsCount; i++)
	{
		Vector3 p1 = lineA;
		Vector3 p2 = lineB;
		Vector3 p3 = points[i];
		
		// have all points on the same Y coordinate
		const float height = i * .5f;
		p1.y = height;
		p2.y = height;
		p3.y = height;

		g_debugRender->AddLine(p1, p2, COLOR_WHITE);
		g_debugRender->AddSphere(p1, .05f, COLOR_RED);
		g_debugRender->AddSphere(p2, .05f, COLOR_BLUE);

		WCHAR msg[32];
		swprintf_s(msg, ARRAYSIZE(msg), L"<%d>", i);
		g_debugRender->AddText(p3, msg, COLOR_BLACK, WithAlpha(COLOR_YELLOW, .75f));
		
		Vector2 testPoints[] = { Vector2(p1.x, p1.z), Vector2(p2.x, p2.z), Vector2(p3.x, p3.z) };
		const bool matchingIsLeft = m_UserWorkSheet->IsLeft(testPoints[0], testPoints[1], testPoints[2]) != m_ControlWorkSheet->IsLeft(testPoints[0], testPoints[1], testPoints[2]);

		// use debug render to draw a red or green check for each test
		g_debugRender->AddTriangle(p1, p2, p3, matchingIsLeft ? Color(0.f, 1.f, .5f, .25f) : Color(1.f, 0.f, .5f, .25f));
		g_debugRender->AddSphere(p3, .05f, matchingIsLeft ? COLOR_GREEN : COLOR_MAGENTA);
	}
}