#pragma once
#include <string>
#include <cmath>

// --------------------
// Basic 2D Math Types
// --------------------
struct Vec2 {
    float x{ 0 };
    float y{ 0 };
};

//inline Vec2 operator-(const Vec2& a, const Vec2& b) {
//    return { a.x - b.x, a.y - b.y };
//}
//inline Vec2 operator+(const Vec2& a, const Vec2& b) {
//    return { a.x + b.x, a.y + b.y };
//}
//inline Vec2 operator*(const Vec2& v, float s) {
//    return { v.x * s, v.y * s };
//}

// --------------------
// Collider Types
// --------------------
struct AABB {
    Vec2 min; // bottom-left
    Vec2 max; // top-right
};

struct Circle {
    Vec2 center;
    float radius;
};

// --------------------
// Static Intersection Tests
// --------------------

// AABB vs AABB
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2);

// Circle vs Circle
bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2);

// AABB vs Circle
bool CollisionIntersection_RectCircle_Static(const AABB& rect, const Circle& circle);

// --------------------
// Dynamic (swept) Intersection Tests
// --------------------

// Moving AABB vs AABB
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
    const Vec2& vel1,
    const AABB& aabb2,
    const Vec2& vel2,
    float& firstTimeOfCollision);

// (Optional: Circle vs Circle dynamic can be added later)

// --------------------
// Unified Helpers
// --------------------

// AABB collision with velocities
inline bool CollisionIntersection_RectRect(const AABB& aabb1,
    const Vec2& vel1,
    const AABB& aabb2,
    const Vec2& vel2,
    float& firstTimeOfCollision)
{
    // Step 1: Static check (already overlapping?)
    if (CollisionIntersection_RectRect_Static(aabb1, aabb2)) {
        firstTimeOfCollision = 0.0f;
        return true;
    }

    // Step 2: Swept test
    return CollisionIntersection_RectRect_Dynamic(aabb1, vel1, aabb2, vel2, firstTimeOfCollision);
}