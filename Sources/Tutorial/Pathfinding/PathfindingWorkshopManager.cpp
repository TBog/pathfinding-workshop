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
#include <random>

using namespace Pathfinding;
//-------------------------------------------------------------------

PathfindingWorkshopManager* PathfindingWorkshopManager::s_Instance = NULL;

PathfindingWorkshopManager::PathfindingWorkshopManager()
	: m_userWorkSheet(new UserPathfindingWorkSheet())
	, m_controlWorkSheet(new ControlPathfindingWorkSheet())
	, m_points(32, 32)
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

int PathfindingWorkshopManager::GetSelectedMenuSubItem() const
{
	const int exerciseIndex = static_cast<int>(m_selectedExercise);
	if (exerciseIndex >= 0 && exerciseIndex < static_cast<int>(std::size(MENU_ITEMS_EXERCISES))) {
		return MENU_ITEMS_EXERCISES[exerciseIndex].subItem;
	}
	else
	{
		myAssert(false, L"Invalid exercise index");
		return 0;
	}
}

//-------------------------------------------------------------------

void PathfindingWorkshopManager::Update(float dt)
{
	// call the function corresponding to the current exercise
	const int exerciseIndex = static_cast<int>(m_selectedExercise);
	if (exerciseIndex >= 0 && exerciseIndex < static_cast<int>(std::size(MENU_ITEMS_EXERCISES))) {
		(this->*MENU_ITEMS_EXERCISES[exerciseIndex].func)();
	}
	else
	{
		myAssert(false, L"Invalid exercise index");
	}

	m_userWorkSheet->Update(dt);
	m_controlWorkSheet->Update(dt);

	const bool pausePressed = g_input->IsKeyPressed(VK_SPACE);
	const bool pauseJustPressed = !m_pausePressed && pausePressed;
	m_pausePressed = pausePressed;

	if (pauseJustPressed)
	{
		m_rotationEnabled = !m_rotationEnabled;
	}

	if (m_rotationEnabled)
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
		const bool leftPressed = g_input->IsKeyPressed(VK_LEFT);
		const bool rightPressed = g_input->IsKeyPressed(VK_RIGHT);
		const bool acceptPressed = g_input->IsKeyPressed(VK_RETURN);
		const bool upJustPressed = !m_upPressed && upPressed;
		const bool downJustPressed = !m_downPressed && downPressed;
		const bool leftJustPressed = !m_leftPressed && leftPressed;
		const bool rightJustPressed = !m_rightPressed && rightPressed;
		const bool acceptJustPressed = !m_acceptPressed && acceptPressed;

		m_upPressed = upPressed;
		m_downPressed = downPressed;
		m_leftPressed = leftPressed;
		m_rightPressed = rightPressed;
		m_acceptPressed = acceptPressed;

		m_menuItem += upJustPressed ? -1 : (downJustPressed ? 1 : 0);
		m_menuItem = (m_menuItem + static_cast<int>(Exercise::_Count)) % static_cast<int>(Exercise::_Count);

		int& menuSubItem = MENU_ITEMS_EXERCISES[m_menuItem].subItem;
		menuSubItem += leftJustPressed ? -1 : (rightJustPressed ? 1 : 0);

		const int subItemCount = MENU_ITEMS_EXERCISES[m_menuItem].subItemCount;
		menuSubItem = subItemCount > 0 ? ((menuSubItem + subItemCount) % subItemCount) : 0;

		if (acceptJustPressed)
		{
			m_selectedExercise = static_cast<Exercise>(m_menuItem);
			m_showDebugMenu = false;
		}

		_DrawDebugMenu();
	}
	else
	{
		if (g_input->IsKeyPressed(VK_ESCAPE))
		{
			m_showDebugMenu = true;
		}

		const int menuItem = static_cast<int>(m_selectedExercise);
		g_debugRender->AddText(10, 10, MENU_ITEMS_EXERCISES[menuItem].name, COLOR_YELLOW, WithAlpha(COLOR_BLACK, .5f));
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
		WCHAR subItems[128] = {0};
		if ((m_menuItem == i) && MENU_ITEMS_EXERCISES[i].subItemCount > 1)
		{
			const wchar_t* subItemOptions[2] = { L"User", L"Control" };
			const int subItemCount = min(MENU_ITEMS_EXERCISES[i].subItemCount, ARRAYSIZE(subItemOptions));

			// Build the subitem string with brackets around the selected one
			wcscpy_s(subItems, L" \u2014 ");
			for (int s = 0; s < subItemCount; ++s)
			{
				if (s > 0)
					wcscat_s(subItems, L" | ");
				if (s == MENU_ITEMS_EXERCISES[i].subItem)
				{
					wcscat_s(subItems, L"<[ ");
					wcscat_s(subItems, subItemOptions[s]);
					wcscat_s(subItems, L" ]>");
				}
				else
				{
					wcscat_s(subItems, subItemOptions[s]);
				}
			}
		}
		WCHAR msg[256];
		swprintf_s(msg, ARRAYSIZE(msg), L"[%lc] %s %s", isSelected ? L'\u2713' : L' ', MENU_ITEMS_EXERCISES[i].name, subItems);
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
		const Vector3 center(-2.f, 0.f, 0.f);
		Vector3 points[pointsCount];
		for (int i = 0; i < pointsCount; i++)
		{
			float angle = (float)i / (float)pointsCount * 2.f * (float)D3DX_PI;
			points[i] = center + Vector3(cosf(angle), 0.f, sinf(angle)) * 3.f;
		}

		const Vector3 lineA = center + Vector3(cosf(m_rotatingAngle) * 5.f, 0.f, sinf(m_rotatingAngle) * 5.f);
		const Vector3 lineB = center + Vector3(cosf(m_rotatingAngle) * -5.f, 0.f, sinf(m_rotatingAngle) * -5.f);

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
		const Vector3 center(6.f, 0.f, 0.f);
		const Vector3 lineA(0.f, 0.f, 0.f);
		const Vector3 lineB(1.f, 0.f, 0.f);
		const int pointsCount = 6;
		const Vector3 points[pointsCount] = { {-1.f, 0.f, -1.f}, {-1.f, 0.f, 0.f}, {-1.f, 0.f, 1.f}, {2.f, 0.f, -1.f}, {2.f, 0.f, 0.f}, {2.f, 0.f, 1.f} };
		for (int i = 0; i < pointsCount; i++)
		{
			Vector3 p1 = center + lineA;
			Vector3 p2 = center + lineB;
			Vector3 p3 = center + points[i];
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
	const Vector2 size(10.f, 10.f);
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
			float x = col * dx - 5.f;
			float y = row * dy - 5.f;
			points.emplace_back(x, y);
		}
	}

	Vector2 triangle[3] = { Vector2(-1.f, -1.f), Vector2(5.f, -1.f), Vector2(2.f, 4.f) };
	// set triangle with the rotation from the previous exercise
	{
		const float angle = m_rotatingAngle;
		const Vector2 center(.5f, .5f);
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
	const Vector2 size(10.f, 10.f);
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
			float x = col * dx - 5.f;
			float y = row * dy - 5.f;
			points.emplace_back(x, y);
		}
	}

	Vector2 triangle[3] = { Vector2(-0.5f, -0.5f), Vector2(-0.5f, 3.2f), Vector2(3.2f, -0.5f) };
	// set triangle with the rotation from the previous exercise
	{
		const float angle = m_rotatingAngle;
		const Vector2 center(0.f, 0.f);
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
		g_debugRender->AddCircle((tB + tC) * .5f, Distance(triangle[1], triangle[2]) * .5f, Vector3(0.f, 1.f, 0.f), WithAlpha(COLOR_BLACK, .25f));
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

// Returns the signed area (shoelace) of a polygon defined by hull indices into points.
// Positive = CCW winding, negative = CW winding.
static float _SignedPolygonArea(const DynVec<Vector2>& points, const DynVec<PointId>& hull)
{
	const int n = hull.GetSize();
	float area = 0.f;
	for (int i = 0; i < n; i++)
	{
		const Vector2& a = points[hull[i]];
		const Vector2& b = points[hull[(i + 1) % n]];
		area += a.x * b.y - b.x * a.y;
	}
	return area * 0.5f;
}

// Draws a red wire-circle at world-space ground position P to flag an error.
static void _MarkErrorPoint(const Vector2& P)
{
	g_debugRender->AddWireCircle(Vector3(P.x, 0.f, P.y), 0.2f, Vector3(0.f, 1.f, 0.f), COLOR_RED);
}

// Validates the user's convex hull and draws red markers for any detected errors:
//   - duplicate consecutive points (including wrap-around from last to first)
//   - input points that lie outside the hull polygon
static void _ValidateConvexHull(const DynVec<Vector2>& points, const DynVec<PointId>& userHull, PathfindingWorkSheet* controlSheet)
{
	const int hullSize = userHull.GetSize();

	// Check for duplicate consecutive points (including last->first wrap-around)
	if (hullSize >= 2)
	{
		for (int i = 0; i < hullSize; i++)
		{
			const PointId curr = userHull[i];
			const PointId next = userHull[(i + 1) % hullSize];
			if (curr == next)
				_MarkErrorPoint(points[curr]);
		}
	}

	// Check that every input point lies inside or on the hull polygon
	if (hullSize >= 3)
	{
		const float polyArea = _SignedPolygonArea(points, userHull);
		if (fabsf(polyArea) > FLT_EPSILON)
		{
			// windingSign > 0 for CCW, < 0 for CW
			const float windingSign = (polyArea > 0.f) ? 1.f : -1.f;

			for (int i = 0; i < points.GetSize(); i++)
			{
				const Vector2& P = points[i];
				bool inside = true;
				for (int j = 0; j < hullSize; j++)
				{
					const Vector2& A = points[userHull[j]];
					const Vector2& B = points[userHull[(j + 1) % hullSize]];
					// Signed area of (A, B, P); negative*windingSign => point is outside edge
					const float signedArea = controlSheet->SignedArea(A, B, P);
					if (signedArea * windingSign < -FLT_EPSILON)
					{
						inside = false;
						break;
					}
				}
				if (!inside)
					_MarkErrorPoint(P);
			}
		}
	}
}

// Validates a user-produced triangulation and draws red markers for detected errors:
//   - missing points (not a vertex of any triangle)      -> red wire circle
//   - degenerate triangles (zero or near-zero area)      -> red wire triangle
//   - wrong/non-reciprocal triangle neighbor links       -> red line from triangle center
//   - overlapping triangles (non-adjacent, vertex inside) -> red filled triangle
static void _ValidateTriangulation(const DynVec<Vector2>& points, const Triangulation& triangulation, PathfindingWorkSheet* controlSheet)
{
	static constexpr float DEGENERATE_AREA_THRESHOLD = 1e-6f;

	// Returns true when P lies strictly inside triangle (a, b, c), excluding boundary.
	auto isStrictlyInside = [&](const Vector2& a, const Vector2& b, const Vector2& c, const Vector2& P) -> bool
	{
		const float s1 = controlSheet->SignedArea(a, b, P);
		const float s2 = controlSheet->SignedArea(b, c, P);
		const float s3 = controlSheet->SignedArea(c, a, P);
		return (s1 < -FLT_EPSILON && s2 < -FLT_EPSILON && s3 < -FLT_EPSILON)
			|| (s1 > FLT_EPSILON && s2 > FLT_EPSILON && s3 > FLT_EPSILON);
	};

	DynVec<Triangle> triangles(max(32, triangulation.GetTriangleCount()), 32);
	triangulation.GetTriangles(triangles);

	const int numTriangles = triangles.GetSize();
	const int numPoints = points.GetSize();

	if (numTriangles == 0)
		return;

	// --- 1. Missing points ---
	std::vector<int> pointUsed(numPoints, 0);
	for (int i = 0; i < numTriangles; i++)
	{
		const Triangle& tri = triangles[i];
		if (tri.p1Id.IsValid() && tri.p1Id < numPoints) pointUsed[tri.p1Id] = 1;
		if (tri.p2Id.IsValid() && tri.p2Id < numPoints) pointUsed[tri.p2Id] = 1;
		if (tri.p3Id.IsValid() && tri.p3Id < numPoints) pointUsed[tri.p3Id] = 1;
	}
	for (int i = 0; i < numPoints; i++)
	{
		if (!pointUsed[i])
			_MarkErrorPoint(points[i]);
	}

	for (int i = 0; i < numTriangles; i++)
	{
		const Triangle& tri = triangles[i];

		if (!tri.p1Id.IsValid() || tri.p1Id >= numPoints) continue;
		if (!tri.p2Id.IsValid() || tri.p2Id >= numPoints) continue;
		if (!tri.p3Id.IsValid() || tri.p3Id >= numPoints) continue;

		const Vector2& tp1 = points[tri.p1Id];
		const Vector2& tp2 = points[tri.p2Id];
		const Vector2& tp3 = points[tri.p3Id];
		const Vector3 v1(tp1.x, 0.f, tp1.y);
		const Vector3 v2(tp2.x, 0.f, tp2.y);
		const Vector3 v3(tp3.x, 0.f, tp3.y);
		const Vector3 triCenter = (v1 + v2 + v3) * (1.f / 3.f);

		// --- 2. Degenerate triangle (zero/near-zero area) ---
		const float area = controlSheet->SignedArea(tp1, tp2, tp3);
		if (fabsf(area) < DEGENERATE_AREA_THRESHOLD)
		{
			g_debugRender->AddWireTriangle(v1, v2, v3, COLOR_RED);
		}

		// --- 3. Wrong / non-reciprocal neighbor links ---
		for (int k = 1; k <= 3; k++)
		{
			const TriangleId nId = tri.GetTriangleId(k);
			if (!nId.IsValid()) continue;

			bool neighborFound = false;
			bool reciprocal = false;
			bool sharesEdge = false;

			for (int j = 0; j < numTriangles; j++)
			{
				if (triangles[j].id != nId) continue;
				neighborFound = true;
				const Triangle& neigh = triangles[j];

				reciprocal = (neigh.t1Id == tri.id || neigh.t2Id == tri.id || neigh.t3Id == tri.id);

				int sharedVerts = 0;
				if (tri.p1Id == neigh.p1Id || tri.p1Id == neigh.p2Id || tri.p1Id == neigh.p3Id) sharedVerts++;
				if (tri.p2Id == neigh.p1Id || tri.p2Id == neigh.p2Id || tri.p2Id == neigh.p3Id) sharedVerts++;
				if (tri.p3Id == neigh.p1Id || tri.p3Id == neigh.p2Id || tri.p3Id == neigh.p3Id) sharedVerts++;
				sharesEdge = (sharedVerts >= 2);
				break;
			}

			if (!neighborFound || !reciprocal || !sharesEdge)
			{
				// Draw a red line from the triangle center toward the bad link direction
				g_debugRender->AddLine(triCenter, triCenter + Vector3(0.f, 0.3f, 0.f), COLOR_RED);
				g_debugRender->AddWireCircle(triCenter, 0.15f, Vector3(0.f, 1.f, 0.f), COLOR_RED);
				break; // one marker per triangle is enough
			}
		}

		// --- 4. Overlapping triangles ---
		// Check if any non-shared vertex of another triangle is strictly inside this one.
		// Uses SignedArea to detect strict interior (excludes boundary/shared edges).
		for (int j = i + 1; j < numTriangles; j++)
		{
			const Triangle& other = triangles[j];

			if (!other.p1Id.IsValid() || other.p1Id >= numPoints) continue;
			if (!other.p2Id.IsValid() || other.p2Id >= numPoints) continue;
			if (!other.p3Id.IsValid() || other.p3Id >= numPoints) continue;

			// Count shared vertices to skip edge-adjacent triangles
			int sharedVerts = 0;
			if (tri.p1Id == other.p1Id || tri.p1Id == other.p2Id || tri.p1Id == other.p3Id) sharedVerts++;
			if (tri.p2Id == other.p1Id || tri.p2Id == other.p2Id || tri.p2Id == other.p3Id) sharedVerts++;
			if (tri.p3Id == other.p1Id || tri.p3Id == other.p2Id || tri.p3Id == other.p3Id) sharedVerts++;
			if (sharedVerts >= 2) continue;

			const Vector2& op1 = points[other.p1Id];
			const Vector2& op2 = points[other.p2Id];
			const Vector2& op3 = points[other.p3Id];

			bool overlap = false;
			// Check if any non-shared vertex of 'other' is strictly inside 'tri'
			for (int v = 1; v <= 3 && !overlap; v++)
			{
				const PointId vid = other.GetPointId(v);
				if (tri.p1Id == vid || tri.p2Id == vid || tri.p3Id == vid) continue;
				if (vid.IsValid() && vid < numPoints)
					overlap = isStrictlyInside(tp1, tp2, tp3, points[vid]);
			}
			// Check if any non-shared vertex of 'tri' is strictly inside 'other'
			for (int v = 1; v <= 3 && !overlap; v++)
			{
				const PointId vid = tri.GetPointId(v);
				if (other.p1Id == vid || other.p2Id == vid || other.p3Id == vid) continue;
				if (vid.IsValid() && vid < numPoints)
					overlap = isStrictlyInside(op1, op2, op3, points[vid]);
			}

			if (overlap)
			{
				const Vector3 ov1(op1.x, 0.f, op1.y);
				const Vector3 ov2(op2.x, 0.f, op2.y);
				const Vector3 ov3(op3.x, 0.f, op3.y);
				g_debugRender->AddTriangle(v1, v2, v3, WithAlpha(COLOR_RED, 0.4f));
				g_debugRender->AddTriangle(ov1, ov2, ov3, WithAlpha(COLOR_RED, 0.4f));
			}
		}
	}
}

void PathfindingWorkshopManager::_RunConvexHullExercise()
{
	const int pointCount = 16;
	DynVec<Vector2> points(32, 32);
	points.Add({ -5.f, -5.f });
	for (int i = 0; i < pointCount; i++)
	{
		float angle = m_rotatingAngle + (float)i / (float)pointCount * 2.f * (float)D3DX_PI;
		float radius = 4.f + cosf(angle * 3.f) * 1.5f;
		points.Add(Vector2(cosf(angle), sinf(angle)) * radius);
	}
	points.Add({ 5.f, 5.f });

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
	_ValidateConvexHull(points, userHull, m_controlWorkSheet);
}

void _DrawTriangles(const Triangulation& triangulation, const Color& wireColor, const Color& triangleColor = COLOR_TRANSPARENT)
{
	const DynVec<Vector2>& points = triangulation.GetPoints();
	DynVec<Triangle> triangles(max(32, triangulation.GetTriangleCount()), 32);
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
	if (triangles.GetSize() == 0)
	{
		const DynVec<Vector2>& points = triangulation.GetPoints();
		for (int i = 0; i < points.GetSize(); i += 1)
		{
			g_debugRender->AddIcosahedron(Vector3(points[i].x, 0.f, points[i].y), .05f, COLOR_WHITE);
		}
	}
}

void _DrawTriangleNeighbors(Triangulation& triangulation, TriangleId triangleId, const Color& bgColor, const Color& fgColor)
{
	const DynVec<Vector2>& points = triangulation.GetPoints();
	DynVec<Triangle> triangles(max(triangulation.GetTriangleCount(), 32), 32);
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
	points.Add({ -5.f, -5.f });
	points.Add({ 5.f, 5.f });
	for (int i = 0; i < pointCount; i++)
	{
		float angle = m_rotatingAngle + (float)i / (float)pointCount * 2.f * (float)D3DX_PI;
		float radius = 4.f + cosf(angle * 3.f) * 1.5f;
		points.Add(Vector2(cosf(angle), sinf(angle)) * radius);
	}

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
	else
	{
		_ValidateTriangulation(points, triangulation, m_controlWorkSheet);
	}

	_DrawTriangles(triangulation, wireColor, WithAlpha(COLOR_BLACK, .25f));
}

void PathfindingWorkshopManager::_RunDelaunayTriangulationExercise()
{
	PathfindingWorkSheet* worksheet = GetSelectedMenuSubItem() == 0 ? m_userWorkSheet : m_controlWorkSheet;

	const int pointCount = 16;
	DynVec<Vector2> points(32, 32);
	points.Add({ -5.f, -5.f });
	points.Add({ 5.f, 5.f });
	for (int i = 0; i < pointCount; i++)
	{
		float angle = m_rotatingAngle + (float)i / (float)pointCount * 2.f * (float)D3DX_PI;
		float radius = 4.f + cosf(angle * 3.f) * 1.5f; // Add some noise to the radius for a more interesting shape
		points.Add(Vector2(cosf(angle), sinf(angle)) * radius);
	}

	for (int i = 0; i < points.GetSize(); i += 1)
	{
		const Vector3 p(points[i].x, 0.f, points[i].y);
		g_debugRender->AddIcosahedron(p, .05f, COLOR_WHITE);
	}

	static int triangleId;
	static int iterations = 0;
	Triangulation triangulation;
	Color wireColor = COLOR_YELLOW;

	worksheet->RandomTriangulation(points, triangulation);
	worksheet->EdgeFlipping(triangulation, iterations);

	if (!m_showDebugMenu)
	{
		const bool upPressed = g_input->IsKeyPressed(VK_UP);
		const bool downPressed = g_input->IsKeyPressed(VK_DOWN);
		const bool leftPressed = g_input->IsKeyPressed(VK_LEFT);
		const bool rightPressed = g_input->IsKeyPressed(VK_RIGHT);
		const bool upJustPressed = !m_upPressed && upPressed;
		const bool downJustPressed = !m_downPressed && downPressed;
		const bool leftJustPressed = !m_leftPressed && leftPressed;
		const bool rightJustPressed = !m_rightPressed && rightPressed;

		m_upPressed = upPressed;
		m_downPressed = downPressed;
		m_leftPressed = leftPressed;
		m_rightPressed = rightPressed;

		if (upJustPressed)
			iterations += 1;
		if (downJustPressed)
			iterations -= (iterations - 1) < 0 ? 0 : 1;
		if (leftJustPressed)
			triangleId -= (triangleId - 1) < -1 ? 0 : 1;
		if (rightJustPressed)
			triangleId += (triangleId + 1) > triangulation.GetTriangleCount() ? 0 : 1;

		WCHAR msg[64];
		swprintf_s(msg, ARRAYSIZE(msg), L"max iterations: %d\ntriangleId: %d", iterations, triangleId);
		g_debugRender->AddText(0, 40, msg, COLOR_WHITE, COLOR_BLACK);

		_DrawTriangles(triangulation, wireColor);
		_DrawTriangleNeighbors(triangulation, TriangleId(triangleId), COLOR_MAGENTA, COLOR_CYAN);
	}
	else
	{
		_DrawTriangles(triangulation, wireColor, WithAlpha(COLOR_BLACK, .25f));
	}
}

void PathfindingWorkshopManager::GenerateRandomPointsAndObstacles(int pointCount, DynVec<Vector2>& obstacleCenters, DynVec<PointId>& obstaclesIndexes, DynVec<Pathfinding::TriangulationConstraint>& constraints)
{
	if (!m_showDebugMenu)
	{
		const bool leftPressed = g_input->IsKeyPressed(VK_LEFT);
		const bool rightPressed = g_input->IsKeyPressed(VK_RIGHT);
		const bool leftJustPressed = !m_leftPressed && leftPressed;
		const bool rightJustPressed = !m_rightPressed && rightPressed;

		m_leftPressed = leftPressed;
		m_rightPressed = rightPressed;

		if (leftJustPressed)
			m_randomSeed -= 1;
		if (rightJustPressed)
			m_randomSeed += 1;
	}

	std::mt19937 rng(m_randomSeed);
	std::uniform_real_distribution<float> distX(-4.9f, 4.9f);
	std::uniform_real_distribution<float> distY(-4.9f, 4.9f);

	m_points.Add({ -5.f, -5.f });
	m_points.Add({ -5.f, 5.f });
	m_points.Add({ 5.f, -5.f });
	m_points.Add({ 5.f, 5.f });
	for (int i = 0; i < pointCount; i++)
	{
		m_points.Add({ distX(rng), distY(rng) });
	}

	const int obstacleCount = 1;
	const int obstacleSize = OBSTACLE_POINT_COUNT;
	const float obstacleRadius = 2.5f;

	for (int x = 0; x < obstacleCount; x++)
	{
		float angle = m_rotatingAngle * .2f + (float)x / (float)obstacleCount * 2.f * (float)D3DX_PI;
		Vector2 center(cosf(angle) * 1.5f, (float)x / (float)obstacleCount * 5.f * ((x % 2) * 2 - 1));
		obstacleCenters.Add(center);

		PointId obstacleStart = PointId(m_points.GetSize());
		obstaclesIndexes.Add(obstacleStart);
		for (int i = 0; i < obstacleSize; i++)
		{
			m_points.Add(center + Vector2(cosf(angle), sinf(angle)) * obstacleRadius);
			angle = angle + (2.f * (float)D3DX_PI) / obstacleSize;

			TriangulationConstraint constraint;
			constraint.p1 = PointId(obstacleStart.value + i);
			constraint.p2 = PointId(obstacleStart.value + (i + 1) % obstacleSize);
			constraints.Add(constraint);
		}

		for (int i = 0; i < obstacleSize; i++)
		{
			TriangulationConstraint& constraint = constraints[constraints.GetSize() - i - 1];
			const Vector3 p1(m_points[constraint.p1].x, 0.f, m_points[constraint.p1].y);
			const Vector3 p2(m_points[constraint.p2].x, 0.f, m_points[constraint.p2].y);
			const Vector3 p3(center.x, 0.f, center.y);
			g_debugRender->AddTriangle(p1, p2, p3, WithAlpha(COLOR_RED, .25f));
		}
	}

	Vector3 p3(2.5f, 0.f, 2.5f);
	for (int i = 0; i < constraints.GetSize(); i += 1)
	{
		const TriangulationConstraint& c = constraints[i];
		const Vector3 p1(m_points[c.p1].x, 0.f, m_points[c.p1].y);
		const Vector3 p2(m_points[c.p2].x, 0.f, m_points[c.p2].y);
		g_debugRender->AddIcosahedron(p1, .05f, COLOR_RED);
		g_debugRender->AddIcosahedron(p2, .05f, COLOR_RED);
	}
}

void PathfindingWorkshopManager::_RunConstrainedDelaunayExercise()
{
	PathfindingWorkSheet* worksheet = GetSelectedMenuSubItem() == 0 ? m_userWorkSheet : m_controlWorkSheet;

	const int pointCount = 28;
	m_points.Clear();
	DynVec<Vector2> obstacleCenters(5, 5);
	DynVec<PointId> obstaclesIndexes(5, 5);
	DynVec<TriangulationConstraint> constraints(25, 32);
	GenerateRandomPointsAndObstacles(pointCount, obstacleCenters, obstaclesIndexes, constraints);

	Triangulation triangulation;

	// Build Triangulation
	Color wireColor = COLOR_YELLOW;
	worksheet->RandomTriangulation(m_points, triangulation);
	worksheet->EdgeFlipping(triangulation);
	worksheet->AddTriangulationConstraints(triangulation, constraints);

	_DrawTriangles(triangulation, wireColor, WithAlpha(COLOR_BLACK, .25f));
}

bool GenerateMaze(const Cell& crt, const Cell& goal, Grid2D<int>& vis, DynVec<Cell>& path, std::mt19937& rng)
{
	constexpr int dx[] = { -1, 1, 0, 0 };
	constexpr int dy[] = { 0, 0, -1, 1 };
	path.Add(crt);
	if (crt.x == goal.x && crt.y == goal.y)
	{
		return true;
	}

	vis[crt.x][crt.y] = 1;

	std::uniform_int_distribution<int> distribution(0, 3);
	int offset = distribution(rng);

	for (int i = 0; i < 4; i++)
	{
		int idx = (i + offset) % 4;
		Cell newCell(crt.x + dx[idx], crt.y + dy[idx]);
		if (newCell.x < 0 || newCell.x >= static_cast<int>(vis.GetRowCount()) || newCell.y < 0 || newCell.y >= static_cast<int>(vis.GetColCount()))
		{
			continue;
		}

		if (vis[newCell.x][newCell.y] == 1)
		{
			continue;
		}

		if (GenerateMaze(newCell, goal, vis, path, rng))
		{
			return true;
		}
	}

	path.Remove(path.GetSize() - 1);
	return false;
}

void PathfindingWorkshopManager::_RunGridPathfindingExercise()
{
	PathfindingWorkSheet* worksheet = GetSelectedMenuSubItem() == 0 ? m_userWorkSheet : m_controlWorkSheet;

	if (!m_showDebugMenu)
	{
		const bool leftPressed = g_input->IsKeyPressed(VK_LEFT);
		const bool rightPressed = g_input->IsKeyPressed(VK_RIGHT);
		const bool leftJustPressed = !m_leftPressed && leftPressed;
		const bool rightJustPressed = !m_rightPressed && rightPressed;

		m_leftPressed = leftPressed;
		m_rightPressed = rightPressed;

		if (leftJustPressed)
			m_randomSeed -= 1;
		if (rightJustPressed)
			m_randomSeed += 1;
	}

	WCHAR msg[64];
	swprintf_s(msg, ARRAYSIZE(msg), L"seed: %u", m_randomSeed);
	g_debugRender->AddText(0, 40, msg, COLOR_WHITE, COLOR_BLACK);

	std::mt19937 rng(m_randomSeed);
	std::uniform_real_distribution<float> distX(-4.9f, 4.9f);
	std::uniform_real_distribution<float> distY(-4.9f, 4.9f);

	const int size = 10;
	DynVec<Vector2> points(32, 32);
	DynVec<int> lineIndexes(32, 32);

	for (int i = 0; i < size + 1; i++)
	{
		for (int j = 0; j < size + 1; j++)
		{
			points.Add(Vector2(i - size / 2 - 0.5f, j - size / 2 - 0.5f));

			int cnt = i * (size + 1) + j;
			if (i < size)
			{
				lineIndexes.Add(cnt);
				lineIndexes.Add(cnt + size + 1);
			}

			if (j < size)
			{
				lineIndexes.Add(cnt);
				lineIndexes.Add(cnt + 1);
			}
		}
	}

	for (int i = 0; i < lineIndexes.GetSize(); i += 2)
	{
		int idx1 = lineIndexes[i];
		int idx2 = lineIndexes[i + 1];
		const Vector3 p1(points[idx1].x, 0.f, points[idx1].y);
		const Vector3 p2(points[idx2].x, 0.f, points[idx2].y);
		g_debugRender->AddLine(p1, p2, COLOR_GREEN);
	}

	Cell start(2, 3);
	Cell goal(7, 8);
	Grid2D<int> map(size, size);
	DynVec<Cell> maze(32, 32);

	GenerateMaze(start, goal, map, maze, rng);

	for (int i = 0; i < size; i++)
	{
		for (int j = 0; j < size; j++)
		{
			map[i][j] = 0;
			if (!maze.Contains(Cell(i, j)))
			{
				map[i][j] = 1;
			}
		}
	}

	DynVec<Cell> gridPath(32, 32);
	worksheet->GridPathfinding(map, start, goal, gridPath);

	g_debugRender->AddCircle({ start.x - size * .5f, 0.f, start.y - size * .5f }, .25f, { 0.f, 1.f, 0.f }, COLOR_RED);
	g_debugRender->AddCircle({ goal.x - size * .5f, 0.f, goal.y - size * .5f }, .25f, { 0.f, 1.f, 0.f }, COLOR_BLUE);

	for (int i = 0; i < gridPath.GetSize() - 1; i++)
	{
		const Vector3 p1(gridPath[i].x - size / 2.f, 0.f, gridPath[i].y - size / 2.f);
		const Vector3 p2(gridPath[i + 1].x - size / 2.f, 0.f, gridPath[i + 1].y - size / 2.f);
		g_debugRender->AddLine(p1, p2, COLOR_CYAN);
		g_debugRender->AddWireCircle(p2, .1f, { 0.f, 1.f, 0.f }, COLOR_CYAN);
	}

	for (int x = 0; x < size; x++)
	{
		for (int y = 0; y < size; y++)
		{
			if (!maze.Contains(Cell(x, y)))
			{
				const Vector3 center(x - size / 2.f, 0.f, y - size / 2.f);
				const Vector3 p1 = center + Vector3(0.4f, 0.f, 0.4f);
				const Vector3 p2 = center + Vector3(0.4f, 0.f, -0.4f);
				const Vector3 p3 = center + Vector3(-0.4f, 0.f, 0.4f);
				const Vector3 p4 = center + Vector3(-0.4f, 0.f, -0.4f);

				g_debugRender->AddTriangle(p1, p2, p3, WithAlpha(COLOR_BLACK, .75f));
				g_debugRender->AddTriangle(p2, p3, p4, WithAlpha(COLOR_BLACK, .75f));
			}
		}
	}
}

bool PathfindingWorkshopManager::IsInsideObstacle(DynVec<Vector2>& obstacleCenters, DynVec<PointId>& obstaclesIndexes, const Vector2& point) const
{
	const int obstacleSize = OBSTACLE_POINT_COUNT;

	for (int i = 0; i < obstaclesIndexes.GetSize(); i++)
	{
		int obstacleStart = obstaclesIndexes[i];
		for (int j = 0; j < obstacleSize; j++)
		{
			Vector2 p1 = m_points[obstacleStart + j];
			Vector2 p2 = m_points[obstacleStart + (j + 1) % obstacleSize];

			if (m_controlWorkSheet->InsideTriangle(obstacleCenters[i], p1, p2, point))
			{
				return true;
			}
		}
	}

	return false;
}

void PathfindingWorkshopManager::_RunAStarPathfindingExercise()
{
	PathfindingWorkSheet* worksheet = GetSelectedMenuSubItem() == 0 ? m_userWorkSheet : m_controlWorkSheet;

	const int pointCount = 28;
	m_points.Clear();
	DynVec<Vector2> obstacleCenters(5, 5);
	DynVec<PointId> obstaclesIndexes(5, 5);
	DynVec<TriangulationConstraint> constraints(25, 32);
	GenerateRandomPointsAndObstacles(pointCount, obstacleCenters, obstaclesIndexes, constraints);

	Triangulation triangulation;

	worksheet->RandomTriangulation(m_points, triangulation);
	worksheet->EdgeFlipping(triangulation);
	worksheet->AddTriangulationConstraints(triangulation, constraints);

	DynVec<Triangle> triangles(max(32, triangulation.GetTriangleCount()), 32);
	triangulation.GetTriangles(triangles);
	for (int i = 0; i < triangles.GetSize(); i++)
	{
		const TriangleId& triId = triangles[i].id;
		const bool isBlocked = IsInsideObstacle(obstacleCenters, obstaclesIndexes, triangulation.GetTriangleCenter(triId));
		triangulation.SetTriangleBlocked(triId, isBlocked);
	}

	triangulation.BuildPointConnectivity();

	_DrawTriangles(triangulation, COLOR_WHITE);

	const float angle = m_rotatingAngle;
	Vector2 startPoint(cosf(angle) - 4.f, sinf(angle) - 4.f);
	Vector2 goalPoint(cosf(angle + PI) + 4.f, sinf(angle + PI) + 4.f);

	DynVec<Vector2> path(32, 32);
	worksheet->AStarPathfinding(triangulation, startPoint, goalPoint, path);

	// draw path
	g_debugRender->AddWireCircle({ startPoint.x, 0.f, startPoint.y }, .25f, { 0.f, 1.f, 0.f }, COLOR_RED);
	g_debugRender->AddWireCircle({ goalPoint.x, 0.f, goalPoint.y }, .25f, { 0.f, 1.f, 0.f }, COLOR_BLUE);

	for (int i = 0; i < path.GetSize() - 1; i += 1)
	{
		const Vector3 p1(path[i].x, 0.f, path[i].y);
		const Vector3 p2(path[i + 1].x, 0.f, path[i + 1].y);
		g_debugRender->AddLine(p1, p2, COLOR_YELLOW);
		g_debugRender->AddWireCircle(p2, .1f, { 0.f, 1.f, 0.f }, COLOR_YELLOW);
	}
}

void PathfindingWorkshopManager::_RunAStarPathfindingSmoothExercise()
{
	PathfindingWorkSheet* worksheet = GetSelectedMenuSubItem() == 0 ? m_userWorkSheet : m_controlWorkSheet;

	const int pointCount = 28;
	m_points.Clear();
	DynVec<Vector2> obstacleCenters(5, 5);
	DynVec<PointId> obstaclesIndexes(5, 5);
	DynVec<TriangulationConstraint> constraints(25, 32);
	GenerateRandomPointsAndObstacles(pointCount, obstacleCenters, obstaclesIndexes, constraints);

	Triangulation triangulation;

	worksheet->RandomTriangulation(m_points, triangulation);
	worksheet->EdgeFlipping(triangulation);
	worksheet->AddTriangulationConstraints(triangulation, constraints);

	DynVec<Triangle> triangles(max(32, triangulation.GetTriangleCount()), 32);
	triangulation.GetTriangles(triangles);
	for (int i = 0; i < triangles.GetSize(); i++)
	{
		const TriangleId& triId = triangles[i].id;
		const bool isBlocked = IsInsideObstacle(obstacleCenters, obstaclesIndexes, triangulation.GetTriangleCenter(triId));
		triangulation.SetTriangleBlocked(triId, isBlocked);
	}

	triangulation.BuildPointConnectivity();

	_DrawTriangles(triangulation, COLOR_WHITE);

	const float angle = m_rotatingAngle;
	Vector2 startPoint(cosf(angle) - 4.f, sinf(angle) - 4.f);
	Vector2 goalPoint(cosf(angle - PI) + 4.f, sinf(angle + PI) + 4.f);

	DynVec<Vector2> path(32, 32);
	worksheet->AStarPathfinding(triangulation, startPoint, goalPoint, path);
	DynVec<Vector2> smoothPath(32, 32);
	worksheet->SmoothPath(triangulation, path, smoothPath);

	// draw smoothPath
	g_debugRender->AddWireCircle({ startPoint.x, 0.f, startPoint.y }, .25f, { 0.f, 1.f, 0.f }, COLOR_RED);
	g_debugRender->AddWireCircle({ goalPoint.x, 0.f, goalPoint.y }, .25f, { 0.f, 1.f, 0.f }, COLOR_BLUE);

	for (int i = 0; i < path.GetSize() - 1; i += 1)
	{
		const Vector3 p1(path[i].x, 0.f, path[i].y);
		const Vector3 p2(path[i + 1].x, 0.f, path[i + 1].y);
		g_debugRender->AddLine(p1, p2, COLOR_YELLOW);
		g_debugRender->AddWireCircle(p2, .1f, { 0.f, 1.f, 0.f }, COLOR_YELLOW);
	}

	for (int i = 0; i < smoothPath.GetSize() - 1; i += 1)
	{
		const Vector3 p1(smoothPath[i].x, 0.01f, smoothPath[i].y);
		const Vector3 p2(smoothPath[i + 1].x, 0.01f, smoothPath[i + 1].y);
		g_debugRender->AddLine(p1, p2, COLOR_GREEN);
		g_debugRender->AddWireCircle(p2, .1f, { 0.f, 1.f, 0.f }, COLOR_GREEN);
	}
}
