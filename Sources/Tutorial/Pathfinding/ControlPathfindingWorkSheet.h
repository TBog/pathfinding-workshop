#pragma once

#include "PathfindingWorkSheet.h"

#include "Utils.h"

#include <algorithm>
#include <queue>
#include <unordered_set>

namespace Pathfinding
{
	class ControlPathfindingWorkSheet : public PathfindingWorkSheet
	{
		struct IndexedPoint {
			Vector2 point;
			PointId idx;
			IndexedPoint(const Vector2& p, PointId i) : point(p), idx(i) {}
			// Sort by x, then y
			bool operator<(const IndexedPoint& other) const {
				if (point.x != other.point.x)
					return point.x < other.point.x;
				return point.y < other.point.y;
			}
		};

		struct QueueEntry {
			Cell cell;
			int parent;
			QueueEntry(const Cell& c, int p) : cell(c), parent(p) {}
		};

		struct AStarNode
		{
			PointId pointId;
			float cost;
			PointId parent;

			AStarNode(PointId pointId, float cost, PointId parent)
				: pointId(pointId), cost(cost), parent(parent)
			{
			}

			// For priority queue (min-heap)
			bool operator>(const AStarNode& other) const { return cost > other.cost; }
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

		void ConvexHull(const DynVec<Vector2>& points, DynVec<PointId>& outHull) override
		{
			const int n = points.GetSize();
			if (n < 3) return;

			std::vector<IndexedPoint> sortedPoints;
			sortedPoints.reserve(n);
			for (int i = 0; i < n; ++i)
				sortedPoints.emplace_back(points[i], PointId(i));

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

		void RandomTriangulation(const DynVec<Vector2>& points, Triangulation& triangulation) override
		{
			// Add all points to triangulation
			for (int i = 0; i < points.GetSize(); ++i)
				triangulation.AddPoint(points[i]);

			// Compute convex hull
			DynVec<PointId> hull(points.GetSize(), 10);
			ConvexHull(points, hull);

			// Track used points
			std::vector<bool> used(points.GetSize(), false);
			for (int i = 0; i < hull.GetSize(); ++i)
				used[hull[i]] = true;

			// Create initial triangles from hull
			TriangleId prevIdx;
			for (int i = 2; i < hull.GetSize(); ++i)
			{
				TriangleId idx = triangulation.AddTriangle(hull[0], hull[i - 1], hull[i]);
				if (prevIdx.IsValid())
					triangulation.LinkTriangles(idx, prevIdx);
				prevIdx = idx;
			}

			// Add remaining points
			for (int i = 0; i < points.GetSize(); ++i)
			{
				if (used[i])
					continue;

				TriangleId trId;
				DynVec<Triangle> triangles(triangulation.GetTriangleCount(), 32);
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

				if (!trId.IsValid())
				{
					WCHAR msg[512];
					swprintf_s(msg, ARRAYSIZE(msg), L"[ControlPathfindingWorkSheet::RandomTriangulation] Didn't find Triangle. Point: %d", i);
					ShowErrorMessageBox(msg);
					continue;
				}

				const Triangle& tri = triangulation.GetTriangle(trId);
				PointId p1Id = tri.p1Id;
				PointId p2Id = tri.p2Id;
				PointId p3Id = tri.p3Id;

				TriangleId t1Id = tri.t1Id;
				TriangleId t2Id = tri.t2Id;
				TriangleId t3Id = tri.t3Id;

				triangulation.RemoveTriangle(trId);

				TriangleId t5 = triangulation.AddTriangle(p1Id, p2Id, PointId(i));
				TriangleId t6 = triangulation.AddTriangle(p2Id, p3Id, PointId(i));
				TriangleId t7 = triangulation.AddTriangle(p3Id, p1Id, PointId(i));

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

		void EdgeFlipping(Triangulation& triangulation, size_t maxIterations) override
		{
			bool swapped = true;
			size_t iterations = 0;
			DynVec<TriangleNeighbourInfo> neighbours(3, 3);

			// Get initial triangle count for kill switch
			DynVec<Triangle> triangles(triangulation.GetTriangleCount(), 32);
			if (maxIterations == 0)
			{
				maxIterations = static_cast<size_t>(triangulation.GetTriangleCount()) * 10;
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
							if (triangulation.FlipTrianglesEdge(t1.id, t2.id))
							{
								swapped = true;
							}
						}
					}
				}
			}
		}

		TriangleId GetConstraintStartTriangle(const Triangulation& triangulation, PointId p1Id, PointId p2Id)
		{
			DynVec<Triangle> triangles(triangulation.GetTriangleCount(), 32);
			triangulation.GetTriangles(triangles);
			for (int i = 0; i < triangles.GetSize(); i++)
			{
				// Do something here            
				if (!triangles[i].IsVertex(p1Id))
				{
					continue;
				}
				PointId tp2Id, tp3Id;
				triangles[i].GetOtherPoints(p1Id, tp2Id, tp3Id);

				Vector2 p1V, p2V, p3V, finalPoint;
				p1V = triangulation.GetPoint(p1Id);
				p2V = triangulation.GetPoint(tp2Id);
				p3V = triangulation.GetPoint(tp3Id);
				finalPoint = triangulation.GetPoint(p2Id);

				float sign = SignedArea(p1V, p2V, p3V);
				if (SignedArea(p1V, p2V, finalPoint) * sign > 0 && SignedArea(p1V, finalPoint, p3V) * sign > 0)
				{
					return triangles[i].id;
				}
			}

			return TriangleId();
		}

		bool TriangulationContainsEdge(const Triangulation& triangulation, PointId p1, PointId p2)
		{
			DynVec<Triangle> triangles(triangulation.GetTriangleCount(), 32);
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

		void AddTriangulationConstraint(Triangulation& triangulation, const TriangulationConstraint& constraint) override
		{
			PointId p1Id = constraint.p1;
			PointId p2Id = constraint.p2;

			int iterations = 0;

			while (p1Id != p2Id && iterations < 1000)
			{
				++iterations;

				// If the edge already exists, nothing to do
				if (TriangulationContainsEdge(triangulation, p1Id, p2Id))
					return;

				// Find the triangle to start from
				TriangleId triangleId = GetConstraintStartTriangle(triangulation, p1Id, p2Id);
				if (!triangleId.IsValid())
					return;

				// Get the two other points of the triangle (not p1Id)
				PointId eP1, eP2;
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

		void GridPathfinding(const Grid2D<int>& map, const Cell& start, const Cell& goal, DynVec<Cell>& outPath) override
		{
			outPath.Clear();

			std::vector<QueueEntry> queue;
			Grid2D<int> visited(map.GetRowCount(), map.GetColCount());
			visited.Fill(0);
			visited[start.x][start.y] = 1;

			static const int dx[4] = { 0, 1, 0, -1 };
			static const int dy[4] = { 1, 0, -1, 0 };

			queue.emplace_back(start, -1);

			for (size_t i = 0; i < queue.size(); ++i)
			{
				const Cell& cell = queue[i].cell;
				for (int k = 0; k < 4; ++k)
				{
					int x = cell.x + dx[k];
					int y = cell.y + dy[k];

					if (x >= 0 && y >= 0 && static_cast<size_t>(x) < map.GetRowCount() && static_cast<size_t>(y) < map.GetColCount() && visited[x][y] == 0 && map[x][y] == 0)
					{
						g_debugRender->AddCircle({ x - map.GetRowCount() * .5f, 0.1f, y - map.GetColCount() * .5f }, .1f, { 0.f, 1.f, 0.f }, COLOR_WHITE);
						visited[x][y] = 1;
						Cell newCell(x, y);
						queue.emplace_back(newCell, static_cast<int>(i));

						if (newCell == goal)
						{
							// Reconstruct path
							outPath.Add(newCell);
							int idx = static_cast<int>(i);
							while (idx >= 0)
							{
								outPath.Add(queue[idx].cell);
								idx = queue[idx].parent;
							}
							// Reverse path
							for (int l = 0, r = outPath.GetSize() - 1; l < r; ++l, --r)
								std::swap(outPath[l], outPath[r]);
							return;
						}
					}
				}
			}
			// No path found, outPath remains empty
		}

		/*void AStarPathfinding(const Triangulation& triangulation, const Vector2& startPoint, const Vector2& goalPoint, DynVec<Vector2>& outPath) override
		{
			int startTriangleId, goalTriangleId;

			startTriangleId = FindTriangle(triangulation, startPoint);
			goalTriangleId = FindTriangle(triangulation, goalPoint);

			std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> queue;
			HashSet<int> visited = new HashSet<int>();

			List<Triangle> triangles = triangulation.triangles;
			List<Vector2> points = triangulation.points;
			List<List<int>> neighbours = triangulation.pointNeighbours;

			points.Add(startPoint);
			int startIdx = points.Count - 1;

			points.Add(goalPoint);
			int goalIdx = points.Count - 1;

			int startP1 = triangles[startTriangleId].p1Id;
			int startP2 = triangles[startTriangleId].p2Id;
			int startP3 = triangles[startTriangleId].p3Id;
			neighbours.Add(new List<int>());
			neighbours.Add(new List<int>());

			neighbours[startIdx].Add(startP1);
			neighbours[startP1].Add(startIdx);

			neighbours[startIdx].Add(startP2);
			neighbours[startP2].Add(startIdx);

			neighbours[startIdx].Add(startP3);
			neighbours[startP3].Add(startIdx);

			int goalP1 = triangles[goalTriangleId].p1Id;
			int goalP2 = triangles[goalTriangleId].p2Id;
			int goalP3 = triangles[goalTriangleId].p3Id;

			neighbours[goalIdx].Add(goalP1);
			neighbours[goalP1].Add(goalIdx);

			neighbours[goalIdx].Add(goalP2);
			neighbours[goalP2].Add(goalIdx);

			neighbours[goalIdx].Add(goalP3);
			neighbours[goalP3].Add(goalIdx);

			queue.Enqueue(new AStarNode(startIdx, 0, -1), 0);
			int[] parent = new int[points.Count];
			float[] minCost = new float[points.Count];

			while (queue.Count > 0)
			{
				AStarNode node = queue.Dequeue();
				int pointId = node.pointId;

				if (visited.Contains(pointId))
				{
					continue;
				}

				parent[pointId] = node.parent;
				minCost[pointId] = node.cost;
				visited.Add(pointId);
				Debug.Log("Expanding Node ID: " + node.pointId.ToString() + " CST: " + node.cost.ToString() + " PRT: " + node.parent.ToString());

				if (pointId == goalIdx)
				{
					break;
				}

				for (int i = 0; i < neighbours[pointId].Count; i++)
				{
					int neigh = neighbours[pointId][i];

					if (visited.Contains(neigh))
					{
						continue;
					}

					float cost;
					int prnt = node.parent;
					if (prnt != -1 && triangulation.CheckLineOfSight(points[neigh], points[prnt]))
					{
						cost = minCost[prnt] + Dist(points[prnt], points[neigh]);
						queue.Enqueue(new AStarNode(neigh, cost, prnt), cost);
						Debug.Log("Add Node ID: " + neigh.ToString() + " CST: " + cost.ToString() + " PRT: " + prnt.ToString());
					}
					else
					{
						cost = node.cost + Dist(points[pointId], points[neigh]);
						queue.Enqueue(new AStarNode(neigh, cost, pointId), cost);
						Debug.Log("Add Node ID: " + neigh.ToString() + " CST: " + cost.ToString() + " PRT: " + pointId.ToString());
					}

				}
			}

			List<Vector2> path = new List<Vector2>();
			int nodeId = goalIdx;
			while (nodeId >= 0)
			{
				path.Add(points[nodeId]);
				nodeId = parent[nodeId];
			}

			return path;
		}*/

		void AStarPathfinding(const Triangulation& triangulation, const Vector2& startPoint, const Vector2& goalPoint, DynVec<Vector2>& outPath) override
		{
			outPath.Clear();

			// Copy points and neighbours for local modification
			DynVec<Vector2> points(triangulation.GetPoints().GetSize(), 32);
			for (int i = 0; i < triangulation.GetPoints().GetSize(); i += 1)
				points.Add(triangulation.GetPoints()[i]);
			std::vector<std::vector<PointId>> neighbours;
			{
				const auto& pn = triangulation.GetNeighbours();
				neighbours.assign(pn.begin(), pn.end());
			}

			// Find triangles containing start and goal
			TriangleId startTriangleId = triangulation.FindTriangle(startPoint);
			TriangleId goalTriangleId = triangulation.FindTriangle(goalPoint);
			if (!startTriangleId.IsValid() || !goalTriangleId.IsValid())
				return;

			// Add start and goal points
			PointId startIdx(points.Add(startPoint));
			PointId goalIdx(points.Add(goalPoint));

			// Expand neighbours for new points
			neighbours.resize(points.GetSize());

			// Connect start point to its triangle's vertices
			const Triangle& startTri = triangulation.GetTriangle(startTriangleId);
			const PointId& startP1 = startTri.p1Id;
			const PointId& startP2 = startTri.p2Id;
			const PointId& startP3 = startTri.p3Id;
			neighbours[startIdx].push_back(startP1); neighbours[startP1].push_back(startIdx);
			neighbours[startIdx].push_back(startP2); neighbours[startP2].push_back(startIdx);
			neighbours[startIdx].push_back(startP3); neighbours[startP3].push_back(startIdx);

			// Connect goal point to its triangle's vertices
			const Triangle& goalTri = triangulation.GetTriangle(goalTriangleId);
			const PointId& goalP1 = goalTri.p1Id;
			const PointId& goalP2 = goalTri.p2Id;
			const PointId& goalP3 = goalTri.p3Id;
			neighbours[goalIdx].push_back(goalP1); neighbours[goalP1].push_back(goalIdx);
			neighbours[goalIdx].push_back(goalP2); neighbours[goalP2].push_back(goalIdx);
			neighbours[goalIdx].push_back(goalP3); neighbours[goalP3].push_back(goalIdx);

			// A* search
			std::priority_queue<AStarNode, std::vector<AStarNode>, std::greater<AStarNode>> queue;
			std::unordered_set<PointId> visited;
			std::vector<PointId> parent(points.GetSize(), PointId());
			std::vector<float> minCost(points.GetSize(), FLT_MAX);

			queue.emplace(startIdx, 0.0f, PointId());

			while (!queue.empty()) {
				AStarNode node = queue.top(); queue.pop();
				const PointId& pointId = node.pointId;

				if (visited.count(pointId)) continue;

				parent[pointId] = node.parent;
				minCost[pointId] = node.cost;
				visited.insert(pointId);

				if (pointId == goalIdx) break;

				for (PointId neigh : neighbours[pointId]) {
					if (visited.count(neigh)) continue;

					float cost;
					PointId prnt = node.parent;
					if (prnt != -1 && triangulation.CheckLineOfSight(points[neigh], points[prnt])) {
						cost = minCost[prnt] + Distance(points[prnt], points[neigh]);
						queue.emplace(neigh, cost, prnt);
					}
					else {
						cost = node.cost + Distance(points[pointId], points[neigh]);
						queue.emplace(neigh, cost, pointId);
					}
				}
			}

			// Reconstruct path
			PointId nodeId = goalIdx;
			while (nodeId >= 0) {
				outPath.Add(points[nodeId]);
				nodeId = parent[nodeId];
			}
			// Reverse path
			for (int l = 0, r = outPath.GetSize() - 1; l < r; ++l, --r)
				std::swap(outPath[l], outPath[r]);
		}

		void SmoothPath(const Triangulation& triangulation, const DynVec<Vector2>& path, DynVec<Vector2>& outPath) override
		{
		}
	};

} //namespace Pathfinding