#include "pch.h"

#include <algorithm>

#include "Triangulation.h"
#include "Utils.h"

namespace Pathfinding
{
	Triangle::Triangle(int _id, int _p1, int _p2, int _p3, int _t1, int _t2, int _t3)
	{
		id = _id;
		p1Id = _p1;
		p2Id = _p2;
		p3Id = _p3;
		t1Id = _t1;
		t2Id = _t2;
		t3Id = _t3;
	}

	Triangle::Triangle(const Triangle& other)
	{
		id   = other.id;
		p1Id = other.p1Id;
		p2Id = other.p2Id;
		p3Id = other.p3Id;
		t1Id = other.t1Id;
		t2Id = other.t2Id;
		t3Id = other.t3Id;
	}

	int Triangle::GetTriangleId(int t) const
	{
		if (t == 1)
		{
			return t1Id;
		}
		else if (t == 2)
		{
			return t2Id;
		}
		else if (t == 3)
		{
			return t3Id;
		}

		return -1;
	}

	int Triangle::GetPointId(int p) const
	{
		if (p == 1)
		{
			return p1Id;
		}
		else if (p == 2)
		{
			return p2Id;
		}
		else if (p == 3)
		{
			return p3Id;
		}

		return -1;
	}

	void Triangle::GetOtherPoints(int p1Id, int& p1, int& p2) const
	{
		int idx = FindVertex(p1Id);
		idx = idx == 3 ? 1 : idx + 1;
		p1 = GetPointId(idx);
		idx = idx == 3 ? 1 : idx + 1;
		p2 = GetPointId(idx);
	}

	int Triangle::GetNeighbourTriangle(int p1Idx, int p2Idx) const
	{
		if (p1Idx > p2Idx)
		{
			Swap(p1Idx, p2Idx);
		}

		if (p1Idx == 1 && p2Idx == 2)
		{
			return t1Id;
		}
		else if (p1Idx == 2 && p2Idx == 3)
		{
			return t2Id;
		}
		else
		{
			return t3Id;
		}
	}

	void Triangle::SetNeighbourTriangle(int p1Idx, int p2Idx, int trIdx)
	{
		if (p1Idx > p2Idx)
		{
			Swap(p1Idx, p2Idx);
		}

		if (p1Idx == 1 && p2Idx == 2)
		{
			t1Id = trIdx;
		}
		else if (p1Idx == 2 && p2Idx == 3)
		{
			t2Id = trIdx;
		}
		else
		{
			t3Id = trIdx;
		}
	}

	int Triangle::FindVertex(int pointId) const
	{
		if (p1Id == pointId)
		{
			return 1;
		}
		else if (p2Id == pointId)
		{
			return 2;
		}
		else if (p3Id == pointId)
		{
			return 3;
		}
		return 0;
	}

	TriangleNeighbourInfo Triangulation::GetTriangleNeighbouringInfo(int t1Idx, int t2Idx) const
	{
		const Triangle& t1 = triangles[t1Idx];
		const Triangle& t2 = triangles[t2Idx];
		myAssert(t1.id == t1Idx, L"Triangle ID mismatch");
		myAssert(t2.id == t2Idx, L"Triangle ID mismatch");

		// find common points
		int cp1 = -1, cp2 = -1;
		int cp1IdxT1 = -1, cp1IdxT2 = -1, cp2IdxT1 = -1, cp2IdxT2 = -1;
		for (int i = 1; i <= 3; i++)
		{
			for (int j = 1; j <= 3; j++)
			{
				if (t1.GetPointId(i) == t2.GetPointId(j))
				{
					if (cp1 == -1)
					{
						cp1 = t1.GetPointId(i);
						cp1IdxT1 = i;
						cp1IdxT2 = j;
					}
					else
					{
						cp2 = t1.GetPointId(i);
						cp2IdxT1 = i;
						cp2IdxT2 = j;
					}
					break;
				}
			}
		}

		if (cp1 == -1 || cp2 == -1)
		{
			WCHAR msg[512];
			swprintf_s(msg, ARRAYSIZE(msg), L"[Triangulation::GetTriangleNeighbouringInfo] Triangles are not neighbours (%d, %d)", t1Idx, t2Idx);
			ShowErrorMessageBox(msg);
			return TriangleNeighbourInfo();
		}

		// find outer-points
		int t1op = -1, t2op = -1;
		int t1opIdx = -1, t2opIdx = -1;
		for (int i = 1; i <= 3; i++)
		{
			if (t1.GetPointId(i) != cp1 && t1.GetPointId(i) != cp2)
			{
				t1op = t1.GetPointId(i);
				t1opIdx = i;
			}

			if (t2.GetPointId(i) != cp1 && t2.GetPointId(i) != cp2)
			{
				t2op = t2.GetPointId(i);
				t2opIdx = i;
			}
		}

		TriangleNeighbourInfo neighbouringInfo;
		neighbouringInfo.neighbourId = t2Idx;
		neighbouringInfo.p1Id = t1op;
		neighbouringInfo.p2Id = cp1;
		neighbouringInfo.p3Id = cp2;
		neighbouringInfo.p4Id = t2op;

		neighbouringInfo.t1Id = t1.GetNeighbourTriangle(t1opIdx, cp1IdxT1);
		neighbouringInfo.t2Id = t1.GetNeighbourTriangle(t1opIdx, cp2IdxT1);

		neighbouringInfo.t3Id = t2.GetNeighbourTriangle(t2opIdx, cp1IdxT2);
		neighbouringInfo.t4Id = t2.GetNeighbourTriangle(t2opIdx, cp2IdxT2);

		return neighbouringInfo;
	}

	void Triangulation::AddTriangleEdge(int t1Idx, int t2Idx, int t1Hint)
	{
		if (t2Idx == -1)
		{
			int t1hint2 = t1Hint == 3 ? 1 : t1Hint + 1;
			triangles[t1Idx].SetNeighbourTriangle(t1Hint, t1hint2, -1);
			return;
		}

		LinkTriangles(t1Idx, t2Idx);
	}

	int Triangulation::AddTriangle(int p1Id, int p2Id, int p3Id)
	{
		if (p1Id < 0 || p1Id >= points.GetSize())
		{
			WCHAR msg[512];
			swprintf_s(msg, ARRAYSIZE(msg), L"[Triangulation::AddTriangle] Invalid Point id: %d. Triangulation contains %d", p1Id, points.GetSize());
			ShowErrorMessageBox(msg);
		}

		if (p2Id < 0 || p2Id >= points.GetSize())
		{
			WCHAR msg[512];
			swprintf_s(msg, ARRAYSIZE(msg), L"[Triangulation::AddTriangle] Invalid Point id: %d. Triangulation contains %d", p2Id, points.GetSize());
			ShowErrorMessageBox(msg);
		}

		if (p3Id < 0 || p3Id >= points.GetSize())
		{
			WCHAR msg[512];
			swprintf_s(msg, ARRAYSIZE(msg), L"[Triangulation::AddTriangle] Invalid Point id: %d. Triangulation contains %d", p3Id, points.GetSize());
			ShowErrorMessageBox(msg);
		}

		Triangle t(-1, p1Id, p2Id, p3Id, -1, -1, -1);

		int tId;
		if (emptyTriangles.GetSize() > 0)
		{
			tId = emptyTriangles[emptyTriangles.GetSize() - 1];
			emptyTriangles.Remove(emptyTriangles.GetSize() - 1);
			triangles[tId] = t;
		}
		else
		{
			triangles.Add(t);
			tId = triangles.GetSize() - 1;
		}

		triangles[tId].id = tId;
		return tId;
	}

	void Triangulation::GetTriangleNeighbours(int tId, DynVec<TriangleNeighbourInfo>& neighbours) const
	{
		neighbours.Clear();
		const Triangle& t = triangles[tId];
		for (int i = 1; i <= 3; i++)
		{
			int n = t.GetTriangleId(i);
			if (n != -1)
			{
				neighbours.Add(GetTriangleNeighbouringInfo(tId, n));
			}
		}
	}

	void Triangulation::GetTriangles(DynVec<Triangle>& ret) const
	{
		ret.Clear();
		for (int i = 0; i < triangles.GetSize(); i++)
		{
			if (triangles[i].id == -1)
			{
				continue;
			}
			ret.Add(triangles[i]);
		}
	}

	void Triangulation::LinkTriangles(int t1Idx, int t2Idx)
	{
		if (t1Idx == -1 || t2Idx == -1)
		{
			return;
		}

		Triangle& t1 = triangles[t1Idx];
		Triangle& t2 = triangles[t2Idx];

		int cp1 = -1, cp2 = -1;
		int cp1IdxT1 = -1, cp1IdxT2 = -1, cp2IdxT1 = -1, cp2IdxT2 = -1;
		for (int i = 1; i <= 3; i++)
		{
			for (int j = 1; j <= 3; j++)
			{
				if (t1.GetPointId(i) == t2.GetPointId(j))
				{
					if (cp1 == -1)
					{
						cp1 = t1.GetPointId(i);
						cp1IdxT1 = i;
						cp1IdxT2 = j;
					}
					else
					{
						cp2 = t1.GetPointId(i);
						cp2IdxT1 = i;
						cp2IdxT2 = j;
					}
					break;
				}
			}
		}

		if (cp1 == -1 || cp2 == -1)
		{
			ShowErrorMessageBox(L"[Triangulation::LinkTriangles] Triangles are not neighbours");
			return;
		}

		t1.SetNeighbourTriangle(cp1IdxT1, cp2IdxT1, t2Idx);
		t2.SetNeighbourTriangle(cp1IdxT2, cp2IdxT2, t1Idx);
	}

	int Triangulation::BreakTriangles(int triangleId, int triangle2Id, Vector2 splitPoint)
	{
		// TODO: assert triangles are neighbours
		TriangleNeighbourInfo neighbourInfo = GetTriangleNeighbouringInfo(triangleId, triangle2Id);

		RemoveTriangle(triangleId);
		RemoveTriangle(neighbourInfo.neighbourId);

		int splitPointId = AddPoint(splitPoint);

		int t1Id = neighbourInfo.t1Id;
		int t2Id = neighbourInfo.t2Id;
		int t3Id = neighbourInfo.t3Id;
		int t4Id = neighbourInfo.t4Id;

		// Create the new triangles
		int t5Id = AddTriangle(neighbourInfo.p1Id, neighbourInfo.p2Id, splitPointId);
		int t6Id = AddTriangle(neighbourInfo.p1Id, neighbourInfo.p3Id, splitPointId);
		int t7Id = AddTriangle(neighbourInfo.p2Id, neighbourInfo.p4Id, splitPointId);
		int t8Id = AddTriangle(neighbourInfo.p3Id, neighbourInfo.p4Id, splitPointId);

		// Add the edges between triangles
		// t5
		AddTriangleEdge(t5Id, t1Id, 1);
		AddTriangleEdge(t5Id, t7Id, 2);
		AddTriangleEdge(t5Id, t6Id, 3);

		// t6
		AddTriangleEdge(t6Id, t2Id, 1);
		AddTriangleEdge(t6Id, t8Id, 2);
		AddTriangleEdge(t6Id, t5Id, 3);

		// t7
		AddTriangleEdge(t7Id, t3Id, 1);
		AddTriangleEdge(t7Id, t8Id, 2);
		AddTriangleEdge(t7Id, t5Id, 3);

		// t8
		AddTriangleEdge(t8Id, t4Id, 1);
		AddTriangleEdge(t8Id, t7Id, 2);
		AddTriangleEdge(t8Id, t6Id, 3);
		return splitPointId;
	}

	void Triangulation::FlipTrianglesEdge(int t1Id, int t2Id)
	{
		const TriangleNeighbourInfo& info = GetTriangleNeighbouringInfo(t1Id, t2Id);

		triangles[t1Id].p1Id = info.p1Id;
		triangles[t1Id].p2Id = info.p2Id;
		triangles[t1Id].p3Id = info.p4Id;

		triangles[t2Id].p1Id = info.p1Id;
		triangles[t2Id].p2Id = info.p3Id;
		triangles[t2Id].p3Id = info.p4Id;

		// update neighbours						
		AddTriangleEdge(t1Id, info.t1Id, 1);
		AddTriangleEdge(t1Id, info.t3Id, 2);
		AddTriangleEdge(t1Id, t2Id, 3);

		AddTriangleEdge(t2Id, info.t2Id, 1);
		AddTriangleEdge(t2Id, info.t4Id, 2);
		AddTriangleEdge(t2Id, t1Id, 3);
	}

	void Triangulation::BuildPointConnectivity()
	{
		// Resize to match the number of points, and clear all inner vectors
		pointNeighbours.clear();
		pointNeighbours.resize(points.GetSize());

		// Build connectivity
		for (int i = 0; i < triangles.GetSize(); i++)
		{
			if (triangles[i].isBlocked)
				continue;

			int p1 = triangles[i].p1Id;
			int p2 = triangles[i].p2Id;
			int p3 = triangles[i].p3Id;

			pointNeighbours[p1].push_back(p2);
			pointNeighbours[p1].push_back(p3);

			pointNeighbours[p2].push_back(p1);
			pointNeighbours[p2].push_back(p3);

			pointNeighbours[p3].push_back(p1);
			pointNeighbours[p3].push_back(p2);
		}

		// Remove duplicates from each neighbor list
		for (auto& neighbors : pointNeighbours)
		{
			std::sort(neighbors.begin(), neighbors.end());
			neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());
		}
	} 

	static float SignedArea(const Vector2& p1, const Vector2& p2, const Vector2& p3)
	{
		return p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.x * p3.y - p3.x * p2.y - p2.x * p1.y;
	}
	
	static bool InsideTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4)
	{
		const float s1 = SignedArea(p1, p2, p4);
		const float s2 = SignedArea(p2, p3, p4);
		const float s3 = SignedArea(p3, p1, p4);

		return (s1 <= 0 && s2 <= 0 && s3 <= 0) || (s1 >= 0 && s2 >= 0 && s3 >= 0);
	}

	int Triangulation::FindTriangle(Vector2 point) const
	{
		for (int i = 0; i < triangles.GetSize(); i += 1)
		{
			const Triangle& tri = triangles[i];
			const Vector2& p1 = points[tri.p1Id];
			const Vector2& p2 = points[tri.p2Id];
			const Vector2& p3 = points[tri.p3Id];

			if (InsideTriangle(p1, p2, p3, point))
			{
				return tri.id;
			}
		}

		return -1;
	}

	bool Triangulation::CheckLineOfSight(Vector2 p1, Vector2 p2) const
	{
		LOSTrianglePath.Clear();

		// bring points a bit closer to break ties when points are part of the triangulation
		p1 = p1 + GetNormalized(p2 - p1) * 0.01f;
		p2 = p2 + GetNormalized(p1 - p2) * 0.01f;

		int tri1Id = FindTriangle(p1);
		int tri2Id = FindTriangle(p2);

		int prevTriangle = -1;

		while (tri1Id != tri2Id)
		{
			LOSTrianglePath.Add(tri1Id);

			if (triangles[tri1Id].isBlocked)
			{
				return false;
			}

			const Vector2& tp1 = points[triangles[tri1Id].p1Id];
			const Vector2& tp2 = points[triangles[tri1Id].p2Id];
			const Vector2& tp3 = points[triangles[tri1Id].p3Id];

			Vector2 inter;
			if (FindSegmentIntersection(p1, p2, tp1, tp2, inter) && triangles[tri1Id].t1Id != prevTriangle)
			{
				prevTriangle = tri1Id;
				tri1Id = triangles[tri1Id].t1Id;
			}
			else if (FindSegmentIntersection(p1, p2, tp2, tp3, inter) && triangles[tri1Id].t2Id != prevTriangle)
			{
				prevTriangle = tri1Id;
				tri1Id = triangles[tri1Id].t2Id;
			}
			else if (FindSegmentIntersection(p1, p2, tp3, tp1, inter) && triangles[tri1Id].t3Id != prevTriangle)
			{
				prevTriangle = tri1Id;
				tri1Id = triangles[tri1Id].t3Id;
			}

			if (LOSTrianglePath.GetSize() > 1000)
			{
				ShowErrorMessageBox(L"LOS Check infinite loop");
				return false;
			}
		}

		LOSTrianglePath.Add(tri1Id);
		return !triangles[tri1Id].isBlocked;
	}

}