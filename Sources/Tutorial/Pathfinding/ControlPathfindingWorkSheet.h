#pragma once

#include "PathfindingWorkSheet.h"

#include "Utils.h"

#include <algorithm>

namespace Pathfinding
{
	class ControlPathfindingWorkSheet : public PathfindingWorkSheet
	{
		struct IndexedPoint {
			Vector2 point;
			int idx;
			IndexedPoint(const Vector2& p, int i) : point(p), idx(i) {}
			// Sort by x, then y
			bool operator<(const IndexedPoint& other) const {
				if (point.x != other.point.x)
					return point.x < other.point.x;
				return point.y < other.point.y;
			}
		};

	public:
		//---------------------------------------------------------------
		// Solution
		//---------------------------------------------------------------

		// Line Side Exercise   
		float SignedArea(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.x * p3.y - p3.x * p2.y - p2.x * p1.y;
		}

		// Line Side Exercise
		bool IsLeft(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return SignedArea(p1, p2, p3) > FLT_EPSILON;
		}

		bool IsCollinear(const Vector2& p1, const Vector2& p2, const Vector2& p3) override
		{
			return fabsf(SignedArea(p1, p2, p3)) <= FLT_EPSILON;
		}

		bool InsideTriangle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) override
		{
			const float s1 = SignedArea(p1, p2, p4);
			const float s2 = SignedArea(p2, p3, p4);
			const float s3 = SignedArea(p3, p1, p4);

			return (s1 <= 0 && s2 <= 0 && s3 <= 0) || (s1 >= 0 && s2 >= 0 && s3 >= 0);
			//return IsLeft(p1, p2, p4) && IsLeft(p2, p3, p4) && IsLeft(p3, p1, p4);
		}

		bool InsideCircumcircle(const Vector2& p1, const Vector2& p2, const Vector2& p3, const Vector2& p4) override
		{
			const float ax = p1.x - p4.x;
			const float bx = p2.x - p4.x;
			const float cx = p3.x - p4.x;
			const float ay = p1.y - p4.y;
			const float by = p2.y - p4.y;
			const float cy = p3.y - p4.y;

			const float val = ax * by * (cx * cx + cy * cy) + bx * cy * (ax * ax + ay * ay) + cx * ay * (bx * bx + by * by) -
				ax * cy * (bx * bx + by * by) - bx * ay * (cx * cx + cy * cy) - cx * by * (ax * ax + ay * ay);

			if (IsLeft(p1, p2, p3))
			{
				return val > 0;
			}
			else
			{
				return val < 0;
			}
		}

		void ConvexHull(const DynVec<Vector2>& points, DynVec<int>& outHull)
		{
			const int n = points.GetSize();
			if (n < 3) return;

			std::vector<IndexedPoint> sortedPoints;
			sortedPoints.reserve(n);
			for (int i = 0; i < n; ++i)
				sortedPoints.emplace_back(points[i], i);

			std::sort(sortedPoints.begin(), sortedPoints.end());

			std::vector<IndexedPoint> hull;
			hull.reserve(2 * n);

			// Lower hull
			for (int i = 0; i < n; ++i) {
				while (hull.size() >= 2 &&
					!IsLeft(hull[hull.size() - 2].point, hull[hull.size() - 1].point, sortedPoints[i].point)) {
					hull.pop_back();
				}
				hull.push_back(sortedPoints[i]);
			}

			// Upper hull
			size_t lowerSize = hull.size();
			for (int i = n - 2; i >= 0; --i) {
				while (hull.size() > lowerSize &&
					!IsLeft(hull[hull.size() - 2].point, hull[hull.size() - 1].point, sortedPoints[i].point)) {
					hull.pop_back();
				}
				hull.push_back(sortedPoints[i]);
				if (i == 0) break; // Prevent underflow for size_t
			}

			// Output indices, skip last point (same as first)
			outHull.Clear();
			for (size_t i = 0; i + 1 < hull.size(); ++i)
				outHull.Add(hull[i].idx);
		}

		void RandomTriangulation(const DynVec<Vector2>& points, Triangulation& triangulation)
		{
			// Add all points to triangulation
			for (int i = 0; i < points.GetSize(); ++i)
				triangulation.AddPoint(points[i]);

			// Compute convex hull
			DynVec<int> hull(points.GetSize(), 10);
			ConvexHull(points, hull);

			// Track used points
			std::vector<bool> used(points.GetSize(), false);
			for (int i = 0; i < hull.GetSize(); ++i)
				used[hull[i]] = true;

			// Create initial triangles from hull
			int prevIdx = -1;
			for (int i = 2; i < hull.GetSize(); ++i)
			{
				int idx = triangulation.AddTriangle(hull[0], hull[i - 1], hull[i]);
				if (prevIdx != -1)
					triangulation.LinkTriangles(idx, prevIdx);
				prevIdx = idx;
			}

			// Add remaining points
			for (int i = 0; i < points.GetSize(); ++i)
			{
				if (used[i])
					continue;

				int trId = -1;
				DynVec<Triangle> triangles(triangulation.GetRegisteredTriangleCount(), 32);
				triangulation.GetTriangles(triangles);
				for (int j = 0; j < triangles.GetSize(); ++j)
				{
					const Triangle& tri = triangles[j];

					Vector2 p1 = triangulation.GetPoint(tri.p1Id);
					Vector2 p2 = triangulation.GetPoint(tri.p2Id);
					Vector2 p3 = triangulation.GetPoint(tri.p3Id);

					if (InsideTriangle(p1, p2, p3, points[i]))
					{
						trId = tri.id;
						break;
					}
				}

				if (trId == -1)
				{
					WCHAR msg[512];
					swprintf_s(msg, ARRAYSIZE(msg), L"[ControlPathfindingWorkSheet::RandomTriangulation] Didn't find Triangle. Point: %d", i);
					ShowErrorMessageBox(msg);
					continue;
				}

				int p1Id = triangles[trId].p1Id;
				int p2Id = triangles[trId].p2Id;
				int p3Id = triangles[trId].p3Id;

				int t1Id = triangles[trId].t1Id;
				int t2Id = triangles[trId].t2Id;
				int t3Id = triangles[trId].t3Id;

				triangulation.RemoveTriangle(trId);

				int t5 = triangulation.AddTriangle(p1Id, p2Id, i);
				int t6 = triangulation.AddTriangle(p2Id, p3Id, i);
				int t7 = triangulation.AddTriangle(p3Id, p1Id, i);

				triangulation.LinkTriangles(t5, t1Id);
				triangulation.LinkTriangles(t5, t6);
				triangulation.LinkTriangles(t5, t7);

				triangulation.LinkTriangles(t6, t2Id);
				triangulation.LinkTriangles(t6, t5);
				triangulation.LinkTriangles(t6, t7);

				triangulation.LinkTriangles(t7, t3Id);
				triangulation.LinkTriangles(t7, t5);
				triangulation.LinkTriangles(t7, t6);
			}
		}

		void EdgeFlipping(Triangulation& triangulation, size_t maxIterations)
		{
			bool swapped = true;
			size_t iterations = 0;
			DynVec<TriangleNeighbourInfo> neighbours(3, 3);

			// Get initial triangle count for kill switch
			DynVec<Triangle> triangles(triangulation.GetRegisteredTriangleCount(), 32);
			if (maxIterations == 0)
			{
				maxIterations = static_cast<size_t>(triangulation.GetRegisteredTriangleCount()) * 10;
			}

			while (swapped && iterations < maxIterations)
			{
				// Iterations is to make sure we stop. If there are 4 points on a circle this algorithm could go endless, iterations acts as a kill switch.
				iterations++;
				swapped = false;

				// Refresh triangle list each iteration
				triangulation.GetTriangles(triangles);

				for (int i = 0; i < triangles.GetSize() && !swapped; ++i)
				{
					const Triangle& tri = triangles[i];
					triangulation.GetTriangleNeighbours(tri.id, neighbours);

					for (int j = 0; j < neighbours.GetSize() && !swapped; ++j)
					{
						const TriangleNeighbourInfo& info = neighbours[j];
						const Triangle& t1 = triangles[i];
						const Triangle& t2 = triangulation.GetTriangle(info.GetNeighbourId());

						const Vector2& p1 = triangulation.GetPoint(info.p1Id);
						const Vector2& p2 = triangulation.GetPoint(info.GetCommonPoint1Id());
						const Vector2& p3 = triangulation.GetPoint(info.GetCommonPoint2Id());
						const Vector2& p4 = triangulation.GetPoint(info.GetNeighbourOuterPointId());

						if (InsideCircumcircle(p1, p2, p3, p4))
						{
							triangulation.FlipTrianglesEdge(t1.id, t2.id);
							swapped = true;
						}
					}
				}
			}
		}

		int GetConstraintStartTriangle(const Triangulation& triangulation, int p1Id, int p2Id)
		{
			DynVec<Triangle> triangles(triangulation.GetRegisteredTriangleCount(), 32);
			triangulation.GetTriangles(triangles);
			for (int i = 0; i < triangles.GetSize(); i++)
			{
				// Do something here            
				if (!triangles[i].IsVertex(p1Id))
				{
					continue;
				}
				int tp2Id = 0, tp3Id = 0;
				triangles[i].GetOtherPoints(p1Id, tp2Id, tp3Id);

				Vector2 p1V, p2V, p3V, finalPoint;
				p1V = triangulation.GetPoint(p1Id);
				p2V = triangulation.GetPoint(tp2Id);
				p3V = triangulation.GetPoint(tp3Id);
				finalPoint = triangulation.GetPoint(p2Id);

				float sign = SignedArea(p1V, p2V, p3V);
				if (SignedArea(p1V, p2V, finalPoint) * sign > 0 && SignedArea(p1V, finalPoint, p3V) * sign > 0)
				{
					return i;
				}
			}

			return -1;
		}

		bool TriangulationContainsEdge(const Triangulation& triangulation, int p1, int p2)
		{
			DynVec<Triangle> triangles(triangulation.GetRegisteredTriangleCount(), 32);
			triangulation.GetTriangles(triangles);
			for (int i = 0; i < triangles.GetSize(); i++)
			{
				bool foundP1 = false;
				bool foundP2 = false;
				if (triangles[i].IsVertex(p1))
				{
					foundP1 = true;
				}

				if (triangles[i].IsVertex(p2))
				{
					foundP2 = true;
				}

				if (foundP1 && foundP2)
				{
					// found a triangle containing both points
					return true;
				}
			}

			return false;
		}

		void AddTriangulationConstraint(Triangulation& triangulation, const TriangulationConstraint& constraint)
		{
			int p1Id = constraint.p1;
			int p2Id = constraint.p2;

			int iterations = 0;

			while (p1Id != p2Id && iterations < 1000)
			{
				++iterations;

				// If the edge already exists, nothing to do
				if (TriangulationContainsEdge(triangulation, p1Id, p2Id))
					return;

				// Find the triangle to start from
				int triangleId = GetConstraintStartTriangle(triangulation, p1Id, p2Id);
				if (triangleId == -1)
					return;

				// Get the two other points of the triangle (not p1Id)
				int eP1 = -1, eP2 = -1;
				triangulation.GetTriangle(triangleId).GetOtherPoints(p1Id, eP1, eP2);

				Vector2 eP1V = triangulation.GetPoint(eP1);
				Vector2 eP2V = triangulation.GetPoint(eP2);

				Vector2 cP1V = triangulation.GetPoint(p1Id);
				Vector2 cP2V = triangulation.GetPoint(p2Id);

				Vector2 intersection;

				// Check for intersection with triangle edges
				if (!FindIntersection(cP1V, cP2V, cP1V, eP1V, intersection))
				{
					p1Id = eP1;
					continue;
				}
				if (!FindIntersection(cP1V, cP2V, cP1V, eP2V, intersection))
				{
					p1Id = eP2;
					continue;
				}
				if (!FindIntersection(cP1V, cP2V, eP1V, eP2V, intersection))
				{
					// Log error if you have a logging system
					// DebugLogError("Something went horribly wrong again");
				}

				// Find the neighbor triangle across the edge starting at p1Id
				DynVec<TriangleNeighbourInfo> triangleNeighboursInfo(4, 4);
				triangulation.GetTriangleNeighbours(triangleId, triangleNeighboursInfo);
				TriangleNeighbourInfo neighbourInfo;
				for (int i = 0; i < triangleNeighboursInfo.GetSize(); ++i)
				{
					if (triangleNeighboursInfo[i].p1Id == p1Id)
					{
						neighbourInfo = triangleNeighboursInfo[i];
						break;
					}
				}

				// Break the triangles and update p1Id to the new intersection point
				p1Id = triangulation.BreakTriangles(triangleId, neighbourInfo.neighbourId, intersection);
			}
		}

		void GridPathfinding(const Grid2D<int>& map, const Cell& start, const Cell& goal, DynVec<Cell>& outPath)
		{
		}

		void AStarPathfinding(const Triangulation& triangulation, const Vector2& startPoint, const Vector2& goalPoint, DynVec<Vector2>& outPath)
		{
		}

		void SmoothPath(const Triangulation& triangulation, const DynVec<Vector2>& path, DynVec<Vector2>& outPath)
		{
		}
	};

} //namespace Pathfinding