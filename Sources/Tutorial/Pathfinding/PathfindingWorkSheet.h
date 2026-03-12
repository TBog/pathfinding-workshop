#pragma once

/// @file PathfindingWorkSheet.h
/// @brief Abstract base class for pathfinding workshop exercises.
///
/// Provides type aliases for common D3DX math types and declares the pure
/// virtual interface that both @ref ControlPathfindingWorkSheet (reference
/// solution) and @ref UserPathfindingWorkSheet (student implementation) must
/// fulfil.

/// Convenience alias for a 2-D floating-point vector.
typedef D3DXVECTOR2 Vector2;
/// Convenience alias for a 3-D floating-point vector.
typedef D3DXVECTOR3 Vector3;
/// Convenience alias for a 4-D floating-point vector.
typedef D3DXVECTOR4 Vector4;
/// Convenience alias for a 4x4 floating-point matrix.
typedef D3DXMATRIX Mat4x4;
/// Convenience alias for an RGBA colour value.
typedef D3DXCOLOR Color;

/// @class PathfindingWorkSheet
/// @brief Abstract interface for pathfinding and geometry exercises.
///
/// Derive from this class to provide either the reference solution
/// (@ref ControlPathfindingWorkSheet) or a student implementation
/// (@ref UserPathfindingWorkSheet).  The @ref PathfindingWorkshopManager drives
/// both instances each frame via @ref Update and @ref Render.
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

	/// @brief Per-frame logic update.
	/// @param dt Elapsed time in seconds since the last frame.
	void Update(float dt) {}

	/// @brief Per-frame draw call.
	void Render() {}

	//---------------------------------------------------------------
	// Exercises
	//---------------------------------------------------------------

	/// @brief Compute the signed area of the triangle formed by three 2-D points.
	///
	/// The return value equals **twice** the signed area of triangle (p1, p2, p3):
	/// - **Positive** — p3 is to the *left* of the directed edge p1→p2
	///   (counter-clockwise winding).
	/// - **Negative** — p3 is to the *right* (clockwise winding).
	/// - **Zero**     — the three points are collinear.
	///
	/// @param p1 First vertex of the triangle.
	/// @param p2 Second vertex of the triangle (defines the directed edge p1→p2).
	/// @param p3 Third vertex to test.
	/// @return Twice the signed area; sign encodes the winding order.
	virtual float SignedArea(Vector2 p1, Vector2 p2, Vector2 p3) = 0;

	/// @brief Test whether a point lies strictly to the left of a directed line.
	///
	/// Returns @c true when @p p3 is strictly to the left of the directed edge
	/// from @p p1 to @p p2 (i.e. @ref SignedArea returns a value greater than
	/// @c FLT_EPSILON).
	///
	/// @param p1 Start point of the directed line.
	/// @param p2 End point of the directed line.
	/// @param p3 Point to classify.
	/// @return @c true if @p p3 is strictly left of p1→p2, @c false otherwise.
	virtual bool IsLeft(Vector2 p1, Vector2 p2, Vector2 p3) = 0;

};
