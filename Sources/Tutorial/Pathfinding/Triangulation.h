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
	protected:
		// Deep copy constructor
		Triangle(const Triangle& other);
	public:

		int GetTriangleId(int t) const;

		int GetPointId(int p) const;

		void GetOtherPoints(int p1Id, int& p1, int& p2) const;

		int GetEdgeNeighbourTriangle(int p1Id, int p2Id) const
		{
			return GetNeighbourTriangle(FindVertex(p1Id), FindVertex(p2Id));
		}

		int GetNeighbourTriangle(int p1Idx, int p2Idx) const;

		void SetNeighbourTriangle(int p1Idx, int p2Idx, int trIdx);

		bool IsVertex(int pointId) const
		{
			return FindVertex(pointId) != 0;
		}

		int FindVertex(int pointId) const; // Rename Find Vertex
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

		int GetNeighbourId() const
		{
			return neighbourId;
		}

		int GetCommonPoint1Id() const
		{
			return p2Id;
		}

		int GetCommonPoint2Id() const
		{
			return p3Id;
		}

		int GetNeighbourOuterPointId() const
		{
			return p4Id;
		}
	};

	struct Triangulation
	{
		DynVec<Vector2> points;
	protected:
		DynVec<Triangle> triangles;
	public:
		std::vector<std::vector<int>> pointNeighbours;
		DynVec<int> emptyTriangles;
		
		Triangulation()
			: points(32, 64)
			, triangles(8, 16)
			, pointNeighbours()
			, emptyTriangles(1, 4)
			, LOSTrianglePath(1, 8)
		{
		}

		// Deep copy constructor
		Triangulation(const Triangulation& other)
			: points(other.points.GetSize(), 64)
			, triangles(other.triangles.GetSize(), 32)
			, pointNeighbours(other.pointNeighbours)
			, emptyTriangles(other.emptyTriangles.GetSize(), 16)
			, LOSTrianglePath(other.LOSTrianglePath.GetSize(), 16)
		{
			for (int i = 0; i < other.points.GetSize(); ++i)
				points.Add(other.points[i]);
			for (int i = 0; i < other.triangles.GetSize(); ++i)
				triangles.Add(other.triangles[i]);
			for (int i = 0; i < other.emptyTriangles.GetSize(); ++i)
				emptyTriangles.Add(other.emptyTriangles[i]);
			for (int i = 0; i < other.LOSTrianglePath.GetSize(); ++i)
				LOSTrianglePath.Add(other.LOSTrianglePath[i]);
		}

		// Deep copy assignment operator
		Triangulation& operator=(const Triangulation& other)
		{
			if (this != &other)
			{
				points.Clear();
				points.SetAllocParams(other.points.GetSize(), other.points.GetSize());
				for (int i = 0; i < other.points.GetSize(); ++i)
					points.Add(other.points[i]);

				triangles.Clear();
				triangles.SetAllocParams(other.triangles.GetSize(), other.triangles.GetSize());
				for (int i = 0; i < other.triangles.GetSize(); ++i)
					triangles.Add(other.triangles[i]);

				pointNeighbours = other.pointNeighbours;

				emptyTriangles.Clear();
				emptyTriangles.SetAllocParams(other.emptyTriangles.GetSize(), other.emptyTriangles.GetSize());
				for (int i = 0; i < other.emptyTriangles.GetSize(); ++i)
					emptyTriangles.Add(other.emptyTriangles[i]);

				LOSTrianglePath.Clear();
				LOSTrianglePath.SetAllocParams(other.LOSTrianglePath.GetSize(), other.LOSTrianglePath.GetSize());
				for (int i = 0; i < other.LOSTrianglePath.GetSize(); ++i)
					LOSTrianglePath.Add(other.LOSTrianglePath[i]);
			}
			return *this;
		}

		TriangleNeighbourInfo GetTriangleNeighbouringInfo(int t1Idx, int t2Idx) const;

		void AddTriangleEdge(int t1Idx, int t2Idx, int t1Hint);

		int AddPoint(const Vector2& p)
		{
			points.Add(p);
			return points.GetSize() - 1;
		}

		const DynVec<Vector2>& GetPoints() const
		{
			return points;
		}

		const Vector2& GetPoint(int pointId) const
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

		const Triangle& GetTriangle(int triangleId) const
		{
			return triangles[triangleId];
		}

		int GetRegisteredTriangleCount() const
		{
			return triangles.GetSize();
		}

		void GetTriangleNeighbours(int tId, DynVec<TriangleNeighbourInfo>& outNeighbours) const;
		void GetTriangles(DynVec<Triangle>& outTriangles) const;
		void LinkTriangles(int t1Idx, int t2Idx);
		int BreakTriangles(int triangleId, int triangle2Id, Vector2 splitPoint);
		void FlipTrianglesEdge(int t1Id, int t2Id);

		Vector2 GetTriangleCenter(int tId) const
		{
			Vector2 p1 = points[GetTriangle(tId).p1Id];
			Vector2 p2 = points[GetTriangle(tId).p2Id];
			Vector2 p3 = points[GetTriangle(tId).p3Id];

			return (p1 + p2 + p3) / 3.f;
		}

		void BuildPointConnectivity();
		int FindTriangle(Vector2 point) const;

		mutable DynVec<int> LOSTrianglePath;
		bool CheckLineOfSight(Vector2 p1, Vector2 p2) const;
	};
}