#pragma once

#include "Types.h"
#include "..\Utils\DynVec.h"

#include <vector>

namespace Pathfinding
{
	struct Triangle
	{
		TriangleId id;

		PointId p1Id;
		PointId p2Id;
		PointId p3Id;

		TriangleId t1Id; // p1-p2
		TriangleId t2Id; // p2-p3
		TriangleId t3Id; // p3-p1

		bool isBlocked = false;

		Triangle(TriangleId _id, PointId _p1, PointId _p2, PointId _p3, TriangleId _t1, TriangleId _t2, TriangleId _t3);
	protected:
		// Deep copy constructor
		Triangle(const Triangle& other);
	public:

		TriangleId GetTriangleId(int t) const;

		PointId GetPointId(int p) const;

		void GetOtherPoints(PointId p1Id, PointId& p1, PointId& p2) const;

		TriangleId GetEdgeNeighbourTriangle(PointId p1Id, PointId p2Id) const
		{
			return GetNeighbourTriangle(FindVertex(p1Id), FindVertex(p2Id));
		}

		TriangleId GetNeighbourTriangle(int p1Idx, int p2Idx) const;

		void SetNeighbourTriangle(int p1Idx, int p2Idx, TriangleId trIdx);

		bool IsVertex(PointId pointId) const
		{
			return FindVertex(pointId) != 0;
		}

		int FindVertex(PointId pointId) const;
	};

	struct TriangleNeighbourInfo
	{
		TriangleId neighbourId;

		// p2-p3 common edge
		PointId p1Id;
		PointId p2Id;
		PointId p3Id;
		PointId p4Id;

		TriangleId t1Id; // p1-p2
		TriangleId t2Id; // p1-p3
		TriangleId t3Id; // p4-p2
		TriangleId t4Id; // p4-p3

		TriangleId GetNeighbourId() const
		{
			return neighbourId;
		}

		PointId GetCommonPoint1Id() const
		{
			return p2Id;
		}

		PointId GetCommonPoint2Id() const
		{
			return p3Id;
		}

		PointId GetNeighbourOuterPointId() const
		{
			return p4Id;
		}
	};

	class Triangulation
	{
		DynVec<Vector2> points;
		DynVec<Triangle> triangles;
		std::vector<std::vector<PointId>> pointNeighbours;
		DynVec<TriangleId> emptyTriangles;
		
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

	public:
		Triangulation()
			: points(32, 64)
			, triangles(8, 16)
			, pointNeighbours()
			, emptyTriangles(1, 4)
			, LOSTrianglePath(1, 8)
		{}

		TriangleNeighbourInfo GetTriangleNeighbouringInfo(TriangleId t1Idx, TriangleId t2Idx) const;

		void AddTriangleEdge(TriangleId t1Idx, TriangleId t2Idx, int t1Hint);

		PointId AddPoint(const Vector2& p)
		{
			points.Add(p);
			return PointId(points.GetSize() - 1);
		}

		const DynVec<Vector2>& GetPoints() const
		{
			return points;
		}

		const Vector2& GetPoint(PointId pointId) const
		{
			return points[pointId];
		}

		TriangleId AddTriangle(PointId p1Id, PointId p2Id, PointId p3Id);

		void RemoveTriangle(TriangleId tId)
		{
			triangles[tId].id = TriangleId(); // invalid id
			emptyTriangles.Add(tId);
		}

		Triangle& GetTriangle(TriangleId triangleId)
		{
			return triangles[triangleId];
		}

		const Triangle& GetTriangle(TriangleId triangleId) const
		{
			return triangles[triangleId];
		}

		int GetTriangleCount() const
		{
			return triangles.GetSize() - emptyTriangles.GetSize();
		}

		void GetTriangleNeighbours(TriangleId tId, DynVec<TriangleNeighbourInfo>& outNeighbours) const;
		void GetTriangles(DynVec<Triangle>& outTriangles) const;
		void LinkTriangles(TriangleId t1Idx, TriangleId t2Idx);
		PointId BreakTriangles(TriangleId triangleId, TriangleId triangle2Id, Vector2 splitPoint);
		bool FlipTrianglesEdge(TriangleId t1Id, TriangleId t2Id);

		Vector2 GetTriangleCenter(TriangleId tId) const
		{
			const Vector2& p1 = points[GetTriangle(tId).p1Id];
			const Vector2& p2 = points[GetTriangle(tId).p2Id];
			const Vector2& p3 = points[GetTriangle(tId).p3Id];

			return (p1 + p2 + p3) / 3.f;
		}

		void BuildPointConnectivity();
		TriangleId FindTriangle(Vector2 point) const;

		mutable DynVec<TriangleId> LOSTrianglePath;
		bool CheckLineOfSight(Vector2 p1, Vector2 p2) const;
	};
}