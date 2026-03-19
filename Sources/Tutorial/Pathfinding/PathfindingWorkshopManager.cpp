#include "pch.h"

#include "PathfindingWorkshopManager.h"
#include "UserPathfindingWorkSheet.h"
#include "ControlPathfindingWorkSheet.h"
#include "Utils.h"

#include "..\Utils\Entity.h"
#include "..\Utils\Utils.h"
#include "..\Utils\Input.h"

#include <algorithm>
#include <cmath>

using namespace Pathfinding;
//-------------------------------------------------------------------

PathfindingWorkshopManager* PathfindingWorkshopManager::s_Instance = NULL;

PathfindingWorkshopManager::PathfindingWorkshopManager()
	: m_userWorkSheet(new UserPathfindingWorkSheet())
	, m_controlWorkSheet(new ControlPathfindingWorkSheet())
{
}

//-------------------------------------------------------------------

PathfindingWorkshopManager::~PathfindingWorkshopManager()
{
	SAFE_DELETE(m_userWorkSheet);
	SAFE_DELETE(m_controlWorkSheet);
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
	switch (m_selectedExercise)
	{
	case Exercise::SignedArea:
		_RunSignedAreaExercise();
		break;
	case Exercise::InsideTriangle:
		_RunInsideTriangleExercise();
		break;
	case Exercise::InsideTriangleCircumcircle:
		_RunInsideTriangleCircumcircleExercise();
		break;
	case Exercise::ConvexHull:
		_RunConvexHullExercise();
		break;
	case Exercise::RandomTriangulation:
		_RunRandomTriangulationExercise();
		break;
	case Exercise::DelaunayTriangulation:
		_RunDelaunayTriangulationExercise();
		break;
	case Exercise::ConstrainedDelaunay:
		_RunConstrainedDelaunayExercise();
		break;
	default:
		break;
	}

	m_userWorkSheet->Update(dt);
	m_controlWorkSheet->Update(dt);

	if (g_input->IsKeyPressed(VK_SPACE))
	{
		m_rotatingAngle += dt * .5f;
	}
	if (m_rotatingAngle > 360.f)
	{
		m_rotatingAngle -= 360.f;
	}

	// handle input for debug menu navigation and selection
	if (m_showDebugMenu)
	{
		const bool upPressed = g_input->IsKeyPressed(VK_UP);
		const bool downPressed = g_input->IsKeyPressed(VK_DOWN);
		const bool acceptPressed = g_input->IsKeyPressed(VK_RETURN);
		const bool upJustPressed = !m_upPressed && upPressed;
		const bool downJustPressed = !m_downPressed && downPressed;
		const bool acceptJustPressed = !m_acceptPressed && acceptPressed;

		m_upPressed = upPressed;
		m_downPressed = downPressed;
		m_acceptPressed = acceptPressed;

		if (upJustPressed)
		{
			m_menuItem -= 1;
		}
		if (downJustPressed)
		{
			m_menuItem += 1;
		}
		if (acceptJustPressed)
		{
			m_selectedExercise = static_cast<Exercise>(m_menuItem);
			m_showDebugMenu = false;
		}

		m_menuItem = (m_menuItem + static_cast<int>(Exercise::_Count)) % static_cast<int>(Exercise::_Count);

		_DrawDebugMenu();
	}
	else
	{
		if (g_input->IsKeyPressed(VK_ESCAPE))
		{
			m_showDebugMenu = true;
		}

		const int menuItem = static_cast<int>(m_selectedExercise);
		g_debugRender->AddText(10, 10, MENU_ITEMS_EXERCISES[menuItem], COLOR_YELLOW, WithAlpha(COLOR_BLACK, .5f));
	}
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::_DrawDebugMenu()
{
	const int menuX = 10;
	int menuY = 10;
	const int lineHeight = 20;
	for (int i = 0; i < ARRAYSIZE(MENU_ITEMS_EXERCISES); i++)
	{
		const bool isSelected = i == static_cast<int>(m_selectedExercise);
		WCHAR msg[32];
		swprintf_s(msg, ARRAYSIZE(msg), L"[%lc] %s", isSelected ? L'\u2713' : L' ', MENU_ITEMS_EXERCISES[i]);
		g_debugRender->AddText(menuX, menuY, msg, COLOR_BLACK, m_menuItem == i ? COLOR_YELLOW : COLOR_WHITE);
		menuY += lineHeight;
	}
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::_RunSignedAreaExercise()
{
	// IsLeft exercise with a rotating line and some generated points in a circle around the line
	{
		const int pointsCount = 8;
		Vector3 points[pointsCount];
		for (int i = 0; i < pointsCount; i++)
		{
			float angle = (float)i / (float)pointsCount * 2.f * (float)D3DX_PI;
			points[i] = Vector3(cosf(angle), 0.f, sinf(angle)) * 3.f;
		}

		const Vector3 lineA(cosf(m_rotatingAngle) * 5.f, 0.f, sinf(m_rotatingAngle) * 5.f);
		const Vector3 lineB(cosf(m_rotatingAngle) * -5.f, 0.f, sinf(m_rotatingAngle) * -5.f);

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
			const bool matchingIsLeft = m_userWorkSheet->IsLeft(testPoints[0], testPoints[1], testPoints[2]) == m_controlWorkSheet->IsLeft(testPoints[0], testPoints[1], testPoints[2]);

			// use debug render to draw a red or green check for each test
			g_debugRender->AddTriangle(p1, p2, p3, matchingIsLeft ? Color(0.f, 1.f, .5f, .25f) : Color(1.f, 0.f, .5f, .25f));
			g_debugRender->AddSphere(p3, .05f, matchingIsLeft ? COLOR_GREEN : COLOR_MAGENTA);
		}
	}

	// IsCollinear exercise with some generated points to the left and some to the right of the line, and some on the line
	{
		const Vector3 lineA(10.f, 0.f, 0.f);
		const Vector3 lineB(11.f, 0.f, 0.f);
		const int pointsCount = 6;
		const Vector3 points[pointsCount] = { {9.f, 0.f, -1.f}, {9.f, 0.f, 0.f}, {9.f, 0.f, 1.f}, {12.f, 0.f, -1.f}, {12.f, 0.f, 0.f}, {12.f, 0.f, 1.f} };
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
			const bool matchingIsCollinear = m_userWorkSheet->IsCollinear(testPoints[0], testPoints[1], testPoints[2]) == m_controlWorkSheet->IsCollinear(testPoints[0], testPoints[1], testPoints[2]);

			// use debug render to draw a red or green check for each test
			g_debugRender->AddTriangle(p1, p2, p3, matchingIsCollinear ? Color(0.f, 1.f, .5f, .25f) : Color(1.f, 0.f, .5f, .25f));
			g_debugRender->AddSphere(p3, .05f, matchingIsCollinear ? COLOR_GREEN : COLOR_MAGENTA);
		}
	}
}

void PathfindingWorkshopManager::_RunInsideTriangleExercise()
{
	const Vector2 size(5.f, 5.f);
	const size_t pointCount = 256;
	std::vector<Vector2> points;
	points.reserve(pointCount);
	// generate non-random points inside the box
	size_t gridCols = static_cast<size_t>(std::ceil(std::sqrt(pointCount)));
	size_t gridRows = static_cast<size_t>(std::ceil(static_cast<float>(pointCount) / gridCols));

	// Compute spacing
	float dx = size.x / (gridCols - 1);
	float dy = size.y / (gridRows - 1);

	for (size_t row = 0; row < gridRows; ++row) {
		for (size_t col = 0; col < gridCols; ++col) {
			if (points.size() >= pointCount)
				break;
			float x = col * dx;
			float y = row * dy;
			points.emplace_back(x, y);
		}
	}

	Vector2 triangle[3] = { Vector2(1.f, 1.f), Vector2(5.f, 1.f), Vector2(2.f, 4.f) };
	// set triangle with the rotation from the previous exercise
	{
		const float angle = m_rotatingAngle;
		const Vector2 center(2.5f, 2.5f);
		for (int i = 0; i < 3; i++)
		{
			Vector2 dir = triangle[i] - center;
			float cosA = cosf(angle);
			float sinA = sinf(angle);
			Vector2 rotatedDir(dir.x * cosA - dir.y * sinA, dir.x * sinA + dir.y * cosA);
			triangle[i] = center + rotatedDir;
		}
	}

	// draw triangle and points, using debug render to draw a red or green check for each point depending on whether the user's implementation of InsideTriangle matches the control implementation
	{
		const Vector3 tA = Vector3(triangle[0].x, 0.f, triangle[0].y);
		const Vector3 tB = Vector3(triangle[1].x, 0.f, triangle[1].y);
		const Vector3 tC = Vector3(triangle[2].x, 0.f, triangle[2].y);
		g_debugRender->AddTriangle(tA, tB, tC, WithAlpha(COLOR_BLACK, .25f));
		g_debugRender->AddLine(tA, tB, COLOR_WHITE);
		g_debugRender->AddLine(tA, tC, COLOR_WHITE);
		g_debugRender->AddLine(tC, tB, COLOR_WHITE);
		for (size_t i = 0; i < pointCount; i++)
		{
			const Vector2& p = points[i];
			const bool insideTriangle = m_controlWorkSheet->InsideTriangle(triangle[0], triangle[1], triangle[2], p);
			const bool matchingInsideTriangle = m_userWorkSheet->InsideTriangle(triangle[0], triangle[1], triangle[2], p) == insideTriangle;
			// use debug render to draw a red or green check for each test
			const float alpha = insideTriangle ? .5f : .2f;
			g_debugRender->AddIcosahedron(Vector3(p.x, 0.f, p.y), .05f, WithAlpha(matchingInsideTriangle ? COLOR_GREEN : COLOR_MAGENTA, alpha));
		}
	}
}

void PathfindingWorkshopManager::_RunInsideTriangleCircumcircleExercise()
{
	const Vector2 size(5.f, 5.f);
	const size_t pointCount = 600;
	std::vector<Vector2> points;
	points.reserve(pointCount);
	// generate non-random points inside the box
	size_t gridCols = static_cast<size_t>(std::ceil(std::sqrt(pointCount)));
	size_t gridRows = static_cast<size_t>(std::ceil(static_cast<float>(pointCount) / gridCols));

	// Compute spacing
	float dx = size.x / (gridCols - 1);
	float dy = size.y / (gridRows - 1);

	for (size_t row = 0; row < gridRows; ++row) {
		for (size_t col = 0; col < gridCols; ++col) {
			if (points.size() >= pointCount)
				break;
			float x = col * dx;
			float y = row * dy;
			points.emplace_back(x, y);
		}
	}

	Vector2 triangle[3] = { Vector2(2.5f, 2.5f), Vector2(2.5f, 4.2f), Vector2(4.2f, 2.5f) };
	// set triangle with the rotation from the previous exercise
	{
		const float angle = m_rotatingAngle;
		const Vector2 center(2.5f, 2.5f);
		for (int i = 0; i < 3; i++)
		{
			Vector2 dir = triangle[i] - center;
			float cosA = cosf(angle);
			float sinA = sinf(angle);
			Vector2 rotatedDir(dir.x * cosA - dir.y * sinA, dir.x * sinA + dir.y * cosA);
			triangle[i] = center + rotatedDir;
		}
	}

	// draw triangle and points, using debug render to draw a red or green check for each point depending on whether the user's implementation of InsideCircumcircle matches the control implementation
	{
		const Vector3 tA = Vector3(triangle[0].x, 0.f, triangle[0].y);
		const Vector3 tB = Vector3(triangle[1].x, 0.f, triangle[1].y);
		const Vector3 tC = Vector3(triangle[2].x, 0.f, triangle[2].y);
		g_debugRender->AddCircle((tB + tC) * .5f, sqrtf(DistSqr(triangle[1], triangle[2])) * .5f, Vector3(0.f, 1.f, 0.f), WithAlpha(COLOR_BLACK, .25f));
		g_debugRender->AddLine(tA, tB, COLOR_WHITE);
		g_debugRender->AddLine(tA, tC, COLOR_WHITE);
		g_debugRender->AddLine(tC, tB, COLOR_WHITE);
		for (size_t i = 0; i < pointCount; i++)
		{
			const Vector2& p = points[i];
			const bool insideCircumcircle = m_controlWorkSheet->InsideCircumcircle(triangle[0], triangle[1], triangle[2], p);
			const bool matchingInsideCircumcircle = m_userWorkSheet->InsideCircumcircle(triangle[0], triangle[1], triangle[2], p) == insideCircumcircle;
			// use debug render to draw a red or green check for each test
			const float alpha = insideCircumcircle ? .5f : .2f;
			g_debugRender->AddIcosahedron(Vector3(p.x, 0.f, p.y), .05f, WithAlpha(matchingInsideCircumcircle ? COLOR_GREEN : COLOR_MAGENTA, alpha));
		}
	}
}

static void _DrawConvexHull(const DynVec<Vector2>& points, const DynVec<PointId>& hull, Color color, float posY = 0.f)
{
	const int hullSize = hull.GetSize();
	for (int i = 0; i < hullSize; i++)
	{
		const Vector2& p1 = points[hull[i]];
		const Vector2& p2 = points[hull[(i + 1) % hullSize]];
		g_debugRender->AddLine(Vector3(p1.x, posY, p1.y), Vector3(p2.x, posY, p2.y), color);
	}
}

void PathfindingWorkshopManager::_RunConvexHullExercise()
{
	const int pointCount = 16;
	DynVec<Vector2> points(32, 32);
	points.Add({0.f, 0.f});
	for (int i = 0; i < pointCount; i++)
	{
		float angle = m_rotatingAngle + (float)i / (float)pointCount * 2.f * (float)D3DX_PI;
		float radius = 2.f + cosf(angle * 3.f) * .5f; // Add some noise to the radius for a more interesting shape
		points.Add(Vector2(cosf(angle), sinf(angle)) * radius + Vector2(2.5f, 2.5f));
	}
	points.Add({5.f, 5.f});

	DynVec<PointId> controlHull(points.GetSize(), 32);
	m_controlWorkSheet->ConvexHull(points, controlHull);
	DynVec<PointId> userHull(points.GetSize(), 32);
	m_userWorkSheet->ConvexHull(points, userHull);

	for (int i = 0; i < points.GetSize(); i += 1)
	{
		const Vector3 p(points[i].x, 0.f, points[i].y);
		g_debugRender->AddIcosahedron(p, .05f, COLOR_WHITE);
	}
	
	_DrawConvexHull(points, userHull, COLOR_YELLOW);
	_DrawConvexHull(points, controlHull, WithAlpha(COLOR_WHITE, 0.5f), .05f);
}

void _DrawTriangles(Triangulation& triangulation, const Color& wireColor, const Color& triangleColor = COLOR_TRANSPARENT)
{
	DynVec<Vector2>& points = triangulation.points;
	DynVec<Triangle> triangles(triangulation.GetTriangleCount(), 32);
	triangulation.GetTriangles(triangles);
	for (int i = 0; i < triangles.GetSize(); i += 1)
	{
		const Triangle& tri = triangles[i];
		const Vector3 p1 = Vector3(points[tri.p1Id].x, 0.f, points[tri.p1Id].y);
		const Vector3 p2 = Vector3(points[tri.p2Id].x, 0.f, points[tri.p2Id].y);
		const Vector3 p3 = Vector3(points[tri.p3Id].x, 0.f, points[tri.p3Id].y);
		if (triangleColor.a > FLT_EPSILON)
		{
			g_debugRender->AddTriangle(p1, p2, p3, triangleColor);
		}
		if (wireColor.a > FLT_EPSILON)
		{
			g_debugRender->AddLine(p1, p2, wireColor);
			g_debugRender->AddLine(p1, p3, wireColor);
			g_debugRender->AddLine(p3, p2, wireColor);
		}
	}
}

void _DrawTriangleNeighbors(Triangulation& triangulation, TriangleId triangleId, const Color& bgColor, const Color& fgColor)
{
	const DynVec<Vector2>& points = triangulation.GetPoints();
	DynVec<Triangle> triangles(triangulation.GetTriangleCount(), 32);
	triangulation.GetTriangles(triangles);
	for (int i = 0; i < triangles.GetSize(); i += 1)
	{
		const Triangle& tri = triangles[i];

		if (triangleId == tri.id)
		{
			g_debugRender->AddTriangle(Vector3(points[tri.p1Id].x, 0.f, points[tri.p1Id].y), Vector3(points[tri.p2Id].x, 0.f, points[tri.p2Id].y), Vector3(points[tri.p3Id].x, 0.f, points[tri.p3Id].y), WithAlpha(bgColor, .5f));
		}

		Vector3 center = Vector3(points[tri.p1Id].x, 0.f, points[tri.p1Id].y) + Vector3(points[tri.p2Id].x, 0.f, points[tri.p2Id].y) + Vector3(points[tri.p3Id].x, 0.f, points[tri.p3Id].y);
		center *= (1.f / 3.f);
		g_debugRender->AddSphere(center, .04f, WithAlpha(bgColor, .25f));
		WCHAR msg[32];
		swprintf_s(msg, ARRAYSIZE(msg), L"[%d]", tri.id.value);
		g_debugRender->AddText(center, msg, fgColor, bgColor);

		if (triangleId == -1 || triangleId == tri.id)
		{
			DynVec<TriangleNeighbourInfo> neighbours(3, 1);
			triangulation.GetTriangleNeighbours(tri.id, neighbours);
			for (int j = 0; j < neighbours.GetSize(); j++)
			{
				const TriangleNeighbourInfo& info = neighbours[j];
				const Triangle& neighborTri = triangulation.GetTriangle(info.neighbourId);
				Vector3 neighborCenter = Vector3(points[neighborTri.p1Id].x, 0.f, points[neighborTri.p1Id].y) + Vector3(points[neighborTri.p2Id].x, 0.f, points[neighborTri.p2Id].y) + Vector3(points[neighborTri.p3Id].x, 0.f, points[neighborTri.p3Id].y);
				neighborCenter *= (1.f / 3.f);
				neighborCenter.y += .05f; // lift neighbor center a bit to prevent z-fighting
				g_debugRender->AddSphere(neighborCenter, .02f, fgColor);
				g_debugRender->AddLine(center, neighborCenter, fgColor);
			}
		}
	}
}

void PathfindingWorkshopManager::_RunRandomTriangulationExercise()
{
	const int pointCount = 16;
	DynVec<Vector2> points(32, 32);
	points.Add({ 0.f, 0.f });
	for (int i = 0; i < pointCount; i++)
	{
		float angle = m_rotatingAngle + (float)i / (float)pointCount * 2.f * (float)D3DX_PI;
		float radius = 2.f + cosf(angle * 3.f) * .5f; // Add some noise to the radius for a more interesting shape
		points.Add(Vector2(cosf(angle), sinf(angle)) * radius + Vector2(2.5f, 2.5f));
	}
	points.Add({ 5.f, 5.f });

	for (int i = 0; i < points.GetSize(); i += 1)
	{
		const Vector3 p(points[i].x, 0.f, points[i].y);
		g_debugRender->AddIcosahedron(p, .05f, COLOR_WHITE);
	}

	Triangulation triangulation;
	m_userWorkSheet->RandomTriangulation(points, triangulation);

	Color wireColor = COLOR_YELLOW;
	if (triangulation.GetTriangleCount() == 0)
	{
		wireColor = COLOR_WHITE;
		m_controlWorkSheet->RandomTriangulation(points, triangulation);
	}
	
	_DrawTriangles(triangulation, wireColor, WithAlpha(COLOR_BLACK, .25f));
}

void PathfindingWorkshopManager::_RunDelaunayTriangulationExercise()
{
	const int pointCount = 16;
	DynVec<Vector2> points(32, 32);
	points.Add({ 0.f, 0.f });
	for (int i = 0; i < pointCount; i++)
	{
		float angle = m_rotatingAngle + (float)i / (float)pointCount * 2.f * (float)D3DX_PI;
		float radius = 2.f + cosf(angle * 3.f) * .5f; // Add some noise to the radius for a more interesting shape
		points.Add(Vector2(cosf(angle), sinf(angle)) * radius + Vector2(2.5f, 2.5f));
	}
	points.Add({ 5.f, 5.f });

	for (int i = 0; i < points.GetSize(); i += 1)
	{
		const Vector3 p(points[i].x, 0.f, points[i].y);
		g_debugRender->AddIcosahedron(p, .05f, COLOR_WHITE);
	}

	static TriangleId triangleId;
	static int iterations = 0;
	Triangulation triangulation;
	Color wireColor = COLOR_YELLOW;

	m_userWorkSheet->RandomTriangulation(points, triangulation);
	m_userWorkSheet->EdgeFlipping(triangulation);

	if (triangulation.GetTriangleCount() == 0)
	{
		wireColor = COLOR_WHITE;
		m_controlWorkSheet->RandomTriangulation(points, triangulation);
		m_controlWorkSheet->EdgeFlipping(triangulation, iterations);

		//static bool m_leftPressed = false;
		//static bool m_rightPressed = false;

		//const bool upPressed = g_input->IsKeyPressed(VK_UP);
		//const bool downPressed = g_input->IsKeyPressed(VK_DOWN);
		//const bool leftPressed = g_input->IsKeyPressed(VK_LEFT);
		//const bool rightPressed = g_input->IsKeyPressed(VK_RIGHT);
		//const bool upJustPressed = !m_upPressed && upPressed;
		//const bool downJustPressed = !m_downPressed && downPressed;
		//const bool leftJustPressed = !m_leftPressed && leftPressed;
		//const bool rightJustPressed = !m_rightPressed && rightPressed;

		//m_upPressed = upPressed;
		//m_downPressed = downPressed;
		//m_leftPressed = leftPressed;
		//m_rightPressed = rightPressed;

		//if (upJustPressed)
		//	iterations += 1;
		//if (downJustPressed)
		//	iterations -= (iterations - 1) < 0 ? 0 : 1;
		//if (leftJustPressed)
		//	triangleId -= (triangleId - 1) < -1 ? 0 : 1;
		//if (rightJustPressed)
		//	triangleId += (triangleId + 1) > triangulation.GetTriangleCount() ? 0 : 1;

		//WCHAR msg[64];
		//swprintf_s(msg, ARRAYSIZE(msg), L"max iterations: %d\ntriangleId: %d", iterations, triangleId);
		//g_debugRender->AddText(0, 40, msg, COLOR_WHITE, COLOR_BLACK);
	}
	
	_DrawTriangles(triangulation, wireColor, WithAlpha(COLOR_BLACK, .25f));
	//_DrawTriangles(triangulation, wireColor);
	//_DrawTriangleNeighbors(triangulation, triangleId, COLOR_GREEN, COLOR_YELLOW);
}

void PathfindingWorkshopManager::_RunConstrainedDelaunayExercise()
{
	const int pointCount = 18;
	DynVec<Vector2> points(32, 32);
	points.Add({ 0.f, 0.f });
	points.Add({ 0.f, 5.f });
	points.Add({ 5.f, 0.f });
	points.Add({ 5.f, 5.f });
	for (int i = 0; i < pointCount; i++)
	{
		float angle = /*m_rotatingAngle*/ + (float)i / (float)pointCount * 2.f * (float)D3DX_PI;
		float radius = 2.f + cosf(angle * 3.f) * .5f;
		points.Add(Vector2(cosf(angle), sinf(angle)) * radius + Vector2(2.5f, 2.5f));
	}

	const int obstacleSize = 5;
	const int obstacleCount = 1;

	DynVec<TriangulationConstraint> constraints(obstacleCount * obstacleSize, 32);

	DynVec<Vector2> obstacleCenters(32, 32);
	DynVec<PointId> obstaclesIndexes(32, 32);
	for (int x = 0; x < obstacleCount; x++)
	{
		float angle = m_rotatingAngle + (float)x / (float)obstacleCount * 2.f * (float)D3DX_PI;
		Vector2 center(2.5f + cosf(angle) * 1.5f, 2.5f);
		obstacleCenters.Add(center);

		PointId obstacleStart = PointId(points.GetSize());
		obstaclesIndexes.Add(obstacleStart);
		for (int i = 0; i < obstacleSize; i++)
		{
			points.Add(center + Vector2(cosf(angle), sinf(angle)));
			angle = angle + (2.f * (float)D3DX_PI) / obstacleSize;

			TriangulationConstraint constraint;
			constraint.p1 = PointId(obstacleStart.value + i);
			constraint.p2 = PointId(obstacleStart.value + (i + 1) % obstacleSize);
			constraints.Add(constraint);
		}
	}

	for (int i = 0; i < constraints.GetSize(); i += 1)
	{
		const TriangulationConstraint& c = constraints[i];
		const Vector3 p1(points[c.p1].x, 0.f, points[c.p1].y);
		const Vector3 p2(points[c.p2].x, 0.f, points[c.p2].y);
		g_debugRender->AddIcosahedron(p1, .05f, COLOR_RED);
		g_debugRender->AddIcosahedron(p2, .05f, COLOR_RED);
	}


	Triangulation triangulation;

	// Build Triangulation
	Color wireColor = COLOR_YELLOW;
	m_userWorkSheet->RandomTriangulation(points, triangulation);
	m_userWorkSheet->EdgeFlipping(triangulation);
	m_userWorkSheet->AddTriangulationConstraints(triangulation, constraints);

	if (triangulation.GetTriangleCount() == 0)
	{
		wireColor = COLOR_WHITE;
		m_controlWorkSheet->RandomTriangulation(points, triangulation);
		m_controlWorkSheet->EdgeFlipping(triangulation);
		m_controlWorkSheet->AddTriangulationConstraints(triangulation, constraints);
	}

	_DrawTriangles(triangulation, wireColor, WithAlpha(COLOR_BLACK, .25f));
}