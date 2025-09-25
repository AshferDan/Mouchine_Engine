#include "collision.h"
#include "GameObject.h"
#include <iostream>
#include <cmath>

//still need to implement dt whenever we make that
// g_fixedDT = ??

// g_dt = ????

// g_appTime = ????

bool CollisionIntersection_RectRect_Static(	const AABB& aabb1,
											const AABB& aabb2)
{
	if
		(
			aabb1.max.x < aabb2.min.x ||
			aabb1.min.x > aabb2.max.x ||
			aabb1.max.y < aabb2.min.y ||
			aabb1.min.y > aabb2.max.y
			)
	{
		return false;
	}
	return true;
};

bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
											const Vec2& vel1,
											const AABB& aabb2,
											const Vec2& vel2,
											float& firstTimeOfCollision)
{
	// relative vector of B to A
	Vec2 Vb = { vel2.x - vel1.x, vel2.y - vel1.y };
	//float dt = (float)g_dt;

	//Time for first collision
	Vec2 tFirst = { 0.f,0.f };

	//Time for last collision
	Vec2 tLast = { 0.f/*dt*/, 0.f/*dt*/}; //Don't forget to replace the 0.f with dt

	// X-axis checks
	if (Vb.x < 0.f)															// If square B is moving left
	{
		if (aabb1.min.x > aabb2.max.x)										// If square B is to the left of A, their x values will not touch
		{
			return false;
		}
		if (aabb1.max.x < aabb2.min.x)										// If square B is to the right of A, their x values will touch
		{
			tFirst.x = max((aabb1.max.x - aabb2.min.x) / Vb.x, tFirst.x);	// Time of first x intersection
			tLast.x = min((aabb1.min.x - aabb2.max.x) / Vb.x, tLast.x);		// Time of last x intersection
		}
	}
	else if (Vb.x > 0.f)													// If square B is moving right
	{
		if (aabb1.min.x > aabb2.max.x)										// If square B is to the left of A, their x values will touch
		{
			tFirst.x = max((aabb1.min.x - aabb2.max.x) / Vb.x, tFirst.x);	// Time of first x intersection
			tLast.x = min((aabb1.max.x - aabb2.min.x) / Vb.x, tLast.x);		// Time of last x intersection
		}
		if (aabb1.max.x < aabb2.min.x)										// If square B is to the right of A, their x values will not touch
		{
			return false;
		}
	}
	else if (Vb.x == 0.f)													// If square B is not moving on the x-axis
	{
		if (aabb1.max.x < aabb2.min.x)										// If square A is to the left of square B
		{
			return false;
		}
		else if (aabb1.min.x > aabb2.max.x)									// If square A is to the right of square B
		{
			return false;
		}
}