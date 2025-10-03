/* Start Header ************************************************************************/
/*!
\file       MathLib.cpp
\author     Lee Hwee Min [100% - 124 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "mathlib.h"
#include <cmath>

// ---------------- vec2 ----------------

vec2 vec2::operator+(vec2 const& v) const noexcept { return { x + v.x, y + v.y }; }
vec2 vec2::operator-(vec2 const& v) const noexcept { return { x - v.x, y - v.y }; }
vec2 vec2::operator*(float s) const noexcept { return { x * s, y * s }; }
vec2 vec2::operator/(float s) const noexcept { return { x / s, y / s }; }

vec2& vec2::operator+=(vec2 const& v) noexcept { x += v.x; y += v.y; return *this; }
vec2& vec2::operator-=(vec2 const& v) noexcept { x -= v.x; y -= v.y; return *this; }
vec2& vec2::operator*=(float s) noexcept { x *= s; y *= s; return *this; }
vec2& vec2::operator/=(float s) noexcept { x /= s; y /= s; return *this; }

float vec2::cross(vec2 const& v) const noexcept { return x * v.y - y * v.x; }
float vec2::length() const noexcept { return std::sqrt(x * x + y * y); }
vec2 vec2::normalized() const noexcept { float len = length(); return len > 0 ? (*this) / len : vec2(0, 0); }
float vec2::angle() const noexcept { return std::atan2(y, x); }

// ---------------- vec3 ----------------
vec3 vec3::operator+(vec3 const& v) const noexcept { return { x + v.x, y + v.y, z + v.z }; }
vec3 vec3::operator-(vec3 const& v) const noexcept { return { x - v.x, y - v.y, z - v.z }; }
vec3 vec3::operator*(float s) const noexcept { return { x * s, y * s, z * s }; }
vec3 vec3::operator/(float s) const noexcept { return { x / s, y / s, z / s }; }

vec3& vec3::operator+=(vec3 const& v) noexcept { x += v.x; y += v.y; z += v.z; return *this; }
vec3& vec3::operator-=(vec3 const& v) noexcept { x -= v.x; y -= v.y; z -= v.z; return *this; }
vec3& vec3::operator*=(float s) noexcept { x *= s; y *= s; z *= s; return *this; }
vec3& vec3::operator/=(float s) noexcept { x /= s; y /= s; z /= s; return *this; }

float vec3::dot(vec3 const& v) const noexcept { return x * v.x + y * v.y + z * v.z; }
vec3 vec3::cross(vec3 const& v) const noexcept { return { y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x }; }
float vec3::length() const noexcept { return std::sqrt(x * x + y * y + z * z); }
vec3 vec3::normalized() const noexcept { float len = length(); return (len > 0) ? (*this) / len : vec3(0, 0, 0); }

// ---------------- vec4 ----------------
vec4 vec4::operator+(vec4 const& v) const noexcept { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
vec4 vec4::operator-(vec4 const& v) const noexcept { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
vec4 vec4::operator*(float s) const noexcept { return { x * s, y * s, z * s, w * s }; }
vec4 vec4::operator/(float s) const noexcept { return { x / s, y / s, z / s, w / s }; }

vec4& vec4::operator+=(vec4 const& v) noexcept { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
vec4& vec4::operator-=(vec4 const& v) noexcept { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
vec4& vec4::operator*=(float s) noexcept { x *= s; y *= s; z *= s; w *= s; return *this; }
vec4& vec4::operator/=(float s) noexcept { x /= s; y /= s; z /= s; w /= s; return *this; }

float vec4::dot(vec4 const& v) const noexcept { return x * v.x + y * v.y + z * v.z + w * v.w; }
float vec4::length() const noexcept { return std::sqrt(x * x + y * y + z * z + w * w); }
vec4 vec4::normalized() const noexcept { float len = length(); return (len > 0) ? (*this) / len : vec4(0, 0, 0, 0); }

// ---------------- mat3 ----------------
mat3::mat3() { for (int i = 0; i < 9; ++i) m[i] = (i % 4 == 0) ? 1.0f : 0.0f; }
mat3 mat3::identity() { return mat3(); }

mat3 mat3::translate(vec2 const& v) { mat3 r = identity(); r.m[6] = v.x; r.m[7] = v.y; return r; }
mat3 mat3::rotate(float rad) { mat3 r = identity(); float c = std::cos(rad), s = std::sin(rad); r.m[0] = c; r.m[3] = -s; r.m[1] = s; r.m[4] = c; return r; }
mat3 mat3::scale(vec2 const& v) { mat3 r = identity(); r.m[0] = v.x; r.m[4] = v.y; return r; }

mat3 mat3::operator*(mat3 const& o) const noexcept {
    mat3 r;
    for (int c = 0; c < 3; ++c)
        for (int rI = 0; rI < 3; ++rI)
            r.m[c * 3 + rI] = m[0 * 3 + rI] * o.m[c * 3 + 0] + m[1 * 3 + rI] * o.m[c * 3 + 1] + m[2 * 3 + rI] * o.m[c * 3 + 2];
    return r;
}

vec2 mat3::operator*(vec2 const& v) const noexcept {
    return { 
        m[0] * v.x + m[3] * v.y + m[6], 
        m[1] * v.x + m[4] * v.y + m[7] 
    };
}

// ---------------- mat4 ----------------
mat4::mat4() { for (int i = 0; i < 16; ++i)m[i] = (i % 5 == 0) ? 1.0f : 0.0f; }
mat4 mat4::identity() { return mat4(); }

mat4 mat4::translate(vec3 const& v) { mat4 r = identity(); r.m[12] = v.x; r.m[13] = v.y; r.m[14] = v.z; return r; }
mat4 mat4::scale(vec3 const& v) { mat4 r = identity(); r.m[0] = v.x; r.m[5] = v.y; r.m[10] = v.z; return r; }

mat4 mat4::rotateX(float rad) { mat4 r = identity(); float c = std::cos(rad), s = std::sin(rad); r.m[5] = c; r.m[9] = -s; r.m[6] = s; r.m[10] = c; return r; }
mat4 mat4::rotateY(float rad) { mat4 r = identity(); float c = std::cos(rad), s = std::sin(rad); r.m[0] = c; r.m[8] = s; r.m[2] = -s; r.m[10] = c; return r; }
mat4 mat4::rotateZ(float rad) { mat4 r = identity(); float c = std::cos(rad), s = std::sin(rad); r.m[0] = c; r.m[4] = -s; r.m[1] = s; r.m[5] = c; return r; }

mat4 mat4::operator*(mat4 const& o) const noexcept {
    mat4 r;
    for (int c = 0; c < 4; ++c)
        for (int rI = 0; rI < 4; ++rI)
            r.m[c * 4 + rI] = m[0 * 4 + rI] * o.m[c * 4 + 0] + m[1 * 4 + rI] * o.m[c * 4 + 1] + m[2 * 4 + rI] * o.m[c * 4 + 2] + m[3 * 4 + rI] * o.m[c * 4 + 3];
    return r;
}

vec3 mat4::operator*(vec3 const& v) const noexcept
{
    return {
        m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12],  
        m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13],  
        m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14]  
    };
}
