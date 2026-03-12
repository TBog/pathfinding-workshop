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

	bool IsCollinear(Vector2 p1, Vector2 p2, Vector2 p3) override
	{
		return fabsf(SignedArea(p1, p2, p3)) <= FLT_EPSILON;
	}

	bool InsideTriangle(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4) override
	{
		const float s1 = SignedArea(p1, p2, p4);
		const float s2 = SignedArea(p2, p3, p4);
		const float s3 = SignedArea(p3, p1, p4);

		return (s1 <= 0 && s2 <= 0 && s3 <= 0) || (s1 >= 0 && s2 >= 0 && s3 >= 0);
		//return IsLeft(p1, p2, p4) && IsLeft(p2, p3, p4) && IsLeft(p3, p1, p4);
	}
};
