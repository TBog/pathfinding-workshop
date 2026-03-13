#pragma once

#include "Types.h"
#include "..\Utils\DynVec.h"

#include <vector>

namespace Pathfinding
{
	struct Triangle
	{
		int id;

		int p1Id;
		int p2Id;
		int p3Id;

		int t1Id; // p1-p2
		int t2Id; // p2-p3
		int t3Id; // p3-p1

		bool isBlocked = false;

		Triangle(int _id, int _p1, int _p2, int _p3, int _t1, int _t2, int _t3);

		int GetTriangleId(int t);

		int GetPointId(int p);

		void GetOtherPoints(int p1Id, int& p1, int& p2);

		int GetEdgeNeighbourTriangle(int p1Id, int p2Id)
		{
			return GetNeighbourTriangle(FindVertex(p1Id), FindVertex(p2Id));
		}

		int GetNeighbourTriangle(int p1Idx, int p2Idx);

		void SetNeighbourTriangle(int p1Idx, int p2Idx, int trIdx);

		bool IsVertex(int pointId)
		{
			return FindVertex(pointId) != 0;
		}

		int FindVertex(int pointId); // Rename Find Vertex
	};

	struct TriangleNeighbourInfo
	{
		int neighbourId;

		// p2-p3 common edge
		int p1Id;
		int p2Id;
		int p3Id;
		int p4Id;

		int t1Id; // p1-p2
		int t2Id; // p1-p3
		int t3Id; // p4-p2
		int t4Id; // p4-p3

		int GetNeighbourId()
		{
			return neighbourId;
		}

		int GetCommonPoint1Id()
		{
			return p2Id;
		}

		int GetCommonPoint2Id()
		{
			return p3Id;
		}

		int GetNeighbourOuterPointId()
		{
			return p4Id;
		}
	};

	struct Triangulation
	{
		DynVec<Vector2> points;
		DynVec<Triangle> triangles;
		std::vector<std::vector<int>> pointNeighbours;
		DynVec<int> emptyTriangles;

		TriangleNeighbourInfo GetTriangleNeighbouringInfo(int t1Idx, int t2Idx);

		void AddTriangleEdge(int t1Idx, int t2Idx, int t1Hint);

		int AddPoint(const Vector2& p)
		{
			points.Add(p);
			return points.GetSize() - 1;
		}

		DynVec<Vector2>& GetPoints()
		{
			return points;
		}

		Vector2 GetPoint(int pointId)
		{
			return points[pointId];
		}

		int AddTriangle(int p1Id, int p2Id, int p3Id);

		void RemoveTriangle(int tId)
		{
			triangles[tId].id = -1;
			emptyTriangles.Add(tId);
		}

		Triangle& GetTriangle(int triangleId)
		{
			return triangles[triangleId];
		}

		void GetTriangleNeighbours(int tId, DynVec<TriangleNeighbourInfo>& outNeighbours);
		void GetTriangles(DynVec<Triangle>& outTriangles);
		void LinkTriangles(int t1Idx, int t2Idx);
		int BreakTriangles(int triangleId, int triangle2Id, Vector2 splitPoint);
		void FlipTrianglesEdge(int t1Id, int t2Id);

		Vector2 GetTriangleCenter(int tId)
		{
			Vector2 p1 = points[GetTriangle(tId).p1Id];
			Vector2 p2 = points[GetTriangle(tId).p2Id];
			Vector2 p3 = points[GetTriangle(tId).p3Id];

			return (p1 + p2 + p3) / 3.f;
		}

		void BuildPointConnectivity();
		int FindTriangle(Vector2 point);

		DynVec<int> LOSTrianglePath;
		bool CheckLineOfSight(Vector2 p1, Vector2 p2);
	};
}