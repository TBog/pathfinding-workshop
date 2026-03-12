#pragma once

/// @file UserPathfindingWorkSheet.h
/// @brief Student implementation stubs for all pathfinding exercises.
///
/// **This is the file you need to edit.**
///
/// Each method below is a stub that currently returns a placeholder value.
/// Replace the placeholder with your own implementation.
/// Compare your results against the reference solution by running the
/// application — both the user and control worksheets are rendered each frame.

#include "PathfindingWorkSheet.h"

/// @class UserPathfindingWorkSheet
/// @brief Student-facing exercise sheet; inherits @ref PathfindingWorkSheet.
///
/// Implement the pure-virtual methods declared in @ref PathfindingWorkSheet.
/// The application will display your results alongside the reference solution
/// so you can verify correctness interactively.
class UserPathfindingWorkSheet : public PathfindingWorkSheet
{
public:
	//---------------------------------------------------------------
	// Exercises
	//---------------------------------------------------------------

	/// @brief **TODO:** Compute the signed area of the triangle (p1, p2, p3).
	///
	/// Return twice the signed area of the triangle formed by the three
	/// 2-D points.  The sign encodes the winding order:
	/// - **Positive** — counter-clockwise (p3 is left of p1→p2).
	/// - **Negative** — clockwise (p3 is right of p1→p2).
	/// - **Zero**     — points are collinear.
	///
	/// @param p1 First vertex.
	/// @param p2 Second vertex (defines directed edge p1→p2).
	/// @param p3 Third vertex to test.
	/// @return Twice the signed area (placeholder returns @c 0).
	float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return false;
	}

	/// @brief **TODO:** Test whether p3 lies strictly to the left of p1→p2.
	///
	/// Use your @ref SignedArea implementation to determine the side.
	///
	/// @param p1 Start point of the directed line.
	/// @param p2 End point of the directed line.
	/// @param p3 Point to classify.
	/// @return @c true if p3 is strictly left of p1→p2 (placeholder returns @c false).
	bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return false;
	}

};