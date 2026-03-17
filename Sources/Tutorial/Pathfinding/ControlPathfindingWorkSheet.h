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
			for (int i = 1; i < hull.GetSize() - 1; ++i)
			{
				int idx = triangulation.AddTriangle(hull[0], hull[i], hull[i + 1]);
				if (prevIdx != -1)
					triangulation.LinkTriangles(idx, prevIdx);
				prevIdx = idx;
			}

			// Add remaining points
			for (int i = 0; i < points.GetSize(); ++i)
			{
				if (used[i])
					continue;

				int trIdx = -1;
				DynVec<Triangle> triangleList(points.GetSize() / 3, points.GetSize() / 3);
				triangulation.GetTriangles(triangleList);
				for (int j = 0; j < triangleList.GetSize(); ++j)
				{
					Vector2 p1 = triangulation.GetPoint(triangleList[j].p1Id);
					Vector2 p2 = triangulation.GetPoint(triangleList[j].p2Id);
					Vector2 p3 = triangulation.GetPoint(triangleList[j].p3Id);
					if (InsideTriangle(p1, p2, p3, points[i]))
					{
						trIdx = j;
						break;
					}
				}

				if (trIdx == -1)
					continue; // Point not inside any triangle

				Triangle backup = triangleList[trIdx];
				triangulation.RemoveTriangle(trIdx);

				int t1Idx = triangulation.AddTriangle(backup.p1Id, backup.p2Id, i);
				int t2Idx = triangulation.AddTriangle(backup.p2Id, backup.p3Id, i);
				int t3Idx = triangulation.AddTriangle(backup.p1Id, backup.p3Id, i);

				// Link triangles
				triangulation.LinkTriangles(t1Idx, backup.t1Id);
				triangulation.LinkTriangles(t1Idx, t2Idx);
				triangulation.LinkTriangles(t1Idx, t3Idx);

				triangulation.LinkTriangles(t2Idx, backup.t2Id);
				triangulation.LinkTriangles(t2Idx, t3Idx);
				triangulation.LinkTriangles(t2Idx, t1Idx);

				triangulation.LinkTriangles(t3Idx, backup.t3Id);
				triangulation.LinkTriangles(t3Idx, t1Idx);
				triangulation.LinkTriangles(t3Idx, t2Idx);
			}
		}

		void EdgeFlipping(const Triangulation& inTriangulation, Triangulation& outTriangulation)
		{
			// Copy input triangulation to output
			outTriangulation = inTriangulation;

			bool swapped = true;
			int iterations = 0;

			// Get initial triangle count for kill switch
			DynVec<Triangle> triangles(outTriangulation.points.GetSize() / 3, outTriangulation.points.GetSize() / 3);
			outTriangulation.GetTriangles(triangles);
			int maxIteration = triangles.GetSize() * 10;

			while (swapped && iterations < maxIteration)
			{
				iterations++;
				swapped = false;

				// Refresh triangle list each iteration
				outTriangulation.GetTriangles(triangles);

				for (int i = 0; i < triangles.GetSize() && !swapped; ++i)
				{
					DynVec<TriangleNeighbourInfo> neighbours(4, 4);
					outTriangulation.GetTriangleNeighbours(i, neighbours);

					for (int j = 0; j < neighbours.GetSize() && !swapped; ++j)
					{
						const Triangle& t1 = triangles[i];
						const Triangle& t2 = outTriangulation.GetTriangle(neighbours[j].GetNeighbourId());

						Vector2 p1 = outTriangulation.GetPoint(t1.p1Id);
						Vector2 p2 = outTriangulation.GetPoint(t1.p2Id);
						Vector2 p3 = outTriangulation.GetPoint(t1.p3Id);
						Vector2 p4 = outTriangulation.GetPoint(neighbours[j].GetNeighbourOuterPointId());

						if (InsideCircumcircle(p1, p2, p3, p4))
						{
							int t1Idx = i;
							int t2Idx = neighbours[j].GetNeighbourId();

							outTriangulation.FlipTrianglesEdge(t1Idx, t2Idx);
							swapped = true;
						}
					}
				}
			}
		}

		int GetConstraintStartTriangle(Triangulation triangulation, int p1Id, int p2Id)
		{
			DynVec<Triangle> triangles(triangulation.triangles.GetSize(), triangulation.triangles.GetSize());
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
			DynVec<Triangle> triangles(triangulation.triangles.GetSize(), triangulation.triangles.GetSize());
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

		void AddTriangulationConstraints(const Triangulation& inTriangulation, const TriangulationConstraint& constraint, Triangulation& outTriangulation)
		{
			// Start with a copy of the input triangulation
			outTriangulation = inTriangulation;

			int p1Id = constraint.p1;
			int p2Id = constraint.p2;

			int iterations = 0;

			// Optional: draw debug line if you have a debug render system
			// DebugLine(outTriangulation.GetPoint(p1Id), outTriangulation.GetPoint(p2Id), COLOR_RED, true, 20);

			while (p1Id != p2Id && iterations < 1000)
			{
				++iterations;

				// If the edge already exists, nothing to do
				if (TriangulationContainsEdge(outTriangulation, p1Id, p2Id))
					return;

				// Find the triangle to start from
				int triangleId = GetConstraintStartTriangle(outTriangulation, p1Id, p2Id);
				if (triangleId == -1)
					return;

				// Get the two other points of the triangle (not p1Id)
				int eP1 = -1, eP2 = -1;
				outTriangulation.GetTriangle(triangleId).GetOtherPoints(p1Id, eP1, eP2);

				Vector2 eP1V = outTriangulation.GetPoint(eP1);
				Vector2 eP2V = outTriangulation.GetPoint(eP2);

				Vector2 cP1V = outTriangulation.GetPoint(p1Id);
				Vector2 cP2V = outTriangulation.GetPoint(p2Id);

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
				outTriangulation.GetTriangleNeighbours(triangleId, triangleNeighboursInfo);
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
				p1Id = outTriangulation.BreakTriangles(triangleId, neighbourInfo.neighbourId, intersection);
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