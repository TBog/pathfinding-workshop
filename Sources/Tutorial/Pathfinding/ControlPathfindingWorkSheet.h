#pragma once

#include "PathfindingWorkSheet.h"

class ControlPathfindingWorkSheet : public PathfindingWorkSheet
{
public:
	//---------------------------------------------------------------
	// Solution
	//---------------------------------------------------------------

	// Line Side Exercise   
	float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return p1.x * p2.y + p2.x * p3.y + p3.x * p1.y - p1.x * p3.y - p3.x * p2.y - p2.x * p1.y;
	}

	// Line Side Exercise
	bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return SignedArea(p1, p2, p3) > FLT_EPSILON;
	}

};
