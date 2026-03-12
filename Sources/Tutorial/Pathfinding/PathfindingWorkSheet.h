#pragma once

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

	// Line Side Exercise   
	virtual float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3) = 0;

	// Line Side Exercise
	virtual bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3) = 0;

};
