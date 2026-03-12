#pragma once

/// @file ControlPathfindingWorkSheet.h
/// @brief Reference (correct) implementations of all pathfinding exercises.
///
/// Do **not** modify this file.  It provides the ground-truth solutions that
/// are displayed alongside the student's output at runtime so that results can
/// be compared interactively.

#include "PathfindingWorkSheet.h"

/// @class ControlPathfindingWorkSheet
/// @brief Reference implementation of @ref PathfindingWorkSheet.
///
/// Contains the correct solutions to all exercises.  Managed by
/// @ref PathfindingWorkshopManager alongside the student's
/// @ref UserPathfindingWorkSheet.
class ControlPathfindingWorkSheet : public PathfindingWorkSheet
{
public:
	//---------------------------------------------------------------
	// Solution
	//---------------------------------------------------------------

	/// @brief Reference implementation: twice the signed area of triangle (p1, p2, p3).
	///
	/// Uses the cross-product expansion:
	/// @code
	/// signed_area = p1.x*p2.y + p2.x*p3.y + p3.x*p1.y
	///             - p1.x*p3.y - p3.x*p2.y - p2.x*p1.y
	/// @endcode
	float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.x * p3.y - p3.x * p2.y - p2.x * p1.y;
	}

	/// @brief Reference implementation: returns @c true if p3 is strictly left of p1→p2.
	bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return SignedArea(p1, p2, p3) > FLT_EPSILON;
	}

};
