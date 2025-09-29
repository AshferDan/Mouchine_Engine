#include "collision.h"
#include <algorithm> // for std::min, std::max
#include <cmath>     // for sqrtf, fmaxf, fminf

// --------------------------------------------------
// Static Intersections
// --------------------------------------------------

// AABB vs AABB
bool CollisionIntersection_RectRect_Static(const AABB& aabb1, const AABB& aabb2)
{
    if (aabb1.max.x < aabb2.min.x) return false;
    if (aabb1.min.x > aabb2.max.x) return false;
    if (aabb1.max.y < aabb2.min.y) return false;
    if (aabb1.min.y > aabb2.max.y) return false;
    return true;
}

// Circle vs Circle
bool CollisionIntersection_CircleCircle_Static(const Circle& c1, const Circle& c2)
{
    float dx = c1.center.x - c2.center.x;
    float dy = c1.center.y - c2.center.y;
    float distSq = dx * dx + dy * dy;
    float radiusSum = c1.radius + c2.radius;
    return distSq <= radiusSum * radiusSum;
}

// AABB vs Circle
bool CollisionIntersection_RectCircle_Static(const AABB& rect, const Circle& circle)
{
    // Clamp circle center to the edges of the rectangle
    float closestX = std::max(rect.min.x, std::min(circle.center.x, rect.max.x));
    float closestY = std::max(rect.min.y, std::min(circle.center.y, rect.max.y));

    float dx = circle.center.x - closestX;
    float dy = circle.center.y - closestY;

    return (dx * dx + dy * dy) <= (circle.radius * circle.radius);
}

// --------------------------------------------------
// Dynamic Intersections
// --------------------------------------------------

// Swept AABB vs AABB
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,
    const Vec2& vel1,
    const AABB& aabb2,
    const Vec2& vel2,
    float& firstTimeOfCollision)
{
    // Relative velocity of B wrt A
    Vec2 relVel = { vel2.x - vel1.x, vel2.y - vel1.y };

    // Initialize times
    float tFirst = 0.0f;
    float tLast = 1.0f; // normalized to frame [0,1]

    // For both axes
    for (int axis = 0; axis < 2; ++axis) {
        float v = (axis == 0 ? relVel.x : relVel.y);
        float aMin = (axis == 0 ? aabb1.min.x : aabb1.min.y);
        float aMax = (axis == 0 ? aabb1.max.x : aabb1.max.y);
        float bMin = (axis == 0 ? aabb2.min.x : aabb2.min.y);
        float bMax = (axis == 0 ? aabb2.max.x : aabb2.max.y);

        if (v < 0.0f) {
            if (aMin > bMax) return false; // moving away
            float tEnter = (aMax - bMin) / v;
            float tExit = (aMin - bMax) / v;
            tFirst = std::max(tFirst, tEnter);
            tLast = std::min(tLast, tExit);
        }
        else if (v > 0.0f) {
            if (aMax < bMin) return false; // moving away
            float tEnter = (aMin - bMax) / v;
            float tExit = (aMax - bMin) / v;
            tFirst = std::max(tFirst, tEnter);
            tLast = std::min(tLast, tExit);
        }
        else {
            // v == 0 ? check overlap
            if (aMax < bMin || aMin > bMax) {
                return false; // no movement, no overlap
            }
        }
    }

    if (tFirst > tLast) {
        return false;
    }

    firstTimeOfCollision = tFirst;
    return true;
}
