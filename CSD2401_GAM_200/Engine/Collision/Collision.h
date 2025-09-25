#pragma once
#include <string>
#include "Component.h"

struct Vec2 {
	float x;
	float y;
};

struct AABB
{
	Vec2 min;
	Vec2 max;
};

bool CollisionIntersection_RectRect_Static( const AABB& aabb1,
											const AABB& aabb2);


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
	//Step 1
	bool staticCollision = false;
	staticCollision = CollisionIntersection_RectRect_Static(aabb1, aabb2);
	if (staticCollision)
	{
		return true;
	}

	//Step 2-5
	return CollisionIntersection_RectRect_Dynamic(aabb1, vel1, aabb2, vel2, firstTimeOfCollision);
}