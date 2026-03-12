#pragma once

#include "..\Render\DebugRender.h"

typedef D3DXVECTOR2 Vector2;
typedef D3DXVECTOR3 Vector3;
typedef D3DXVECTOR4 Vector4;
typedef D3DXMATRIX Mat4x4;
typedef D3DXCOLOR Color;

class PathfindingWorkSheet
{
public:
	//---------------------------------------------------------------
	//	CONSTRUCTOR / DESTRUCTOR
	//---------------------------------------------------------------
	PathfindingWorkSheet() {}
	virtual ~PathfindingWorkSheet() {}

	//---------------------------------------------------------------
	//	MAIN FUNCTIONS
	//---------------------------------------------------------------
	void Update(float dt) {}
	void Render() {}

	//---------------------------------------------------------------
	// Exercises
	//---------------------------------------------------------------

	// Exercise 1: Signed Area

	virtual float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3) = 0;
	virtual bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3) = 0;
	virtual bool IsCollinear(Vector2 p1, Vector2 p2, Vector2 p3) = 0;

	// Exercise 2: Inside Triangle
	virtual bool InsideTriangle(Vector2 p1, Vector2 p2, Vector2 p3, Vector2 p4) = 0;
};
