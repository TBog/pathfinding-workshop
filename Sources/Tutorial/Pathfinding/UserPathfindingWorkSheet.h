#pragma once

#include "PathfindingWorkSheet.h"

class UserPathfindingWorkSheet : public PathfindingWorkSheet
{
public:
	//---------------------------------------------------------------
	// Exercises
	//---------------------------------------------------------------

	// Line Side Exercise   
	float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return false;
	}

	// Line Side Exercise
	bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return false;
	}

};