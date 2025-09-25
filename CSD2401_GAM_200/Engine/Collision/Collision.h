#pragma once
#include <string>
#include "Component.h"

//vector
struct Vec2 {
	float x;
	float y;
};

//defining min vector (bottom-left) max vector (top-right)
struct AABB
{
	Vec2 min;
	Vec2 max;
};

//Fixed Rectangle
bool CollisionIntersection_RectRect_Static( const AABB& aabb1,
											const AABB& aabb2);

//Moving Rectangle
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,					
											const Vec2& vel1,					 
											const AABB& aabb2,					
											const Vec2& vel2,					
											float& firstTimeOfCollision);

inline bool CollisionIntersection_RectRect(	const AABB& aabb1,
											const Vec2& vel1,
											const AABB& aabb2,
											const Vec2& vel2,
											float& firstTimeOfCollision)
{
	// Step 1
	// First check "Step 1", is the static intersection check => return intersection 
	// in case both rectangles are already overlapping.
	bool staticCollision = false;
	staticCollision = CollisionIntersection_RectRect_Static(aabb1, aabb2);
	if (staticCollision)
	{
		return true;
	}

	// Step 2-5
	// in case the first check fails, we apply a more
	// dynamic collision test to see if any rectangle's movement may lead to intersection 
	// in this game loop.
	// returns true if there is intersection.
	return CollisionIntersection_RectRect_Dynamic(aabb1, vel1, aabb2, vel2, firstTimeOfCollision);
}