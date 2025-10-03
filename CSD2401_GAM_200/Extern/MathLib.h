/* Start Header ************************************************************************/
/*!
\file       MathLib.h
\author     Lee Hwee Min [100% - 67 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef MATHLIB_H
#define MATHLIB_H

#include <cmath>
#include <cstdint>

// ---------------- vec2 ----------------
/**
 * @brief Represents a two-dimensional vector with components x and y.
 *        Provides common vector operations such as addition, subtraction, scaling,
 *        dot product, cross product, length calculation, angle computation, and normalization.
 */
struct vec2 {
    float x, y;
    vec2(float X = 0, float Y = 0) : x(X), y(Y) {}

    vec2 operator+(vec2 const& v) const noexcept;
    vec2 operator-(vec2 const& v) const noexcept;
    vec2 operator*(float s) const noexcept;
    vec2 operator/(float s) const noexcept;

    vec2& operator+=(vec2 const& v) noexcept;
    vec2& operator-=(vec2 const& v) noexcept;
    vec2& operator*=(float s) noexcept;
    vec2& operator/=(float s) noexcept;

    float dot(vec2 const& v) const noexcept;
    float cross(vec2 const& v) const noexcept;
    float length() const noexcept;
    vec2 normalized() const noexcept;
    float angle() const noexcept;
};

// ---------------- vec3 ----------------
/**
 * @brief Represents a three-dimensional vector with components x, y, and z.
 *        Provides common vector operations such as addition, subtraction, scaling,
 *        dot product, cross product, length calculation, and normalization.
 */
struct vec3 {
    float x, y, z;
    vec3(float X = 0, float Y = 0, float Z = 0) : x(X), y(Y), z(Z) {}

    vec3 operator+(vec3 const& v) const noexcept;
    vec3 operator-(vec3 const& v) const noexcept;
    vec3 operator*(float s) const noexcept;
    vec3 operator/(float s) const noexcept;

    vec3& operator+=(vec3 const& v) noexcept;
    vec3& operator-=(vec3 const& v) noexcept;
    vec3& operator*=(float s) noexcept;
    vec3& operator/=(float s) noexcept;

    float dot(vec3 const& v) const noexcept;
    vec3 cross(vec3 const& v) const noexcept;
    float length() const noexcept;
    vec3 normalized() const noexcept;
};

// ---------------- vec4 ----------------
/**
 * @brief Represents a four-dimensional vector with components x, y, z, and w.
 *        Provides common vector operations such as addition, subtraction, scaling,
 *        dot product, length calculation, and normalization.
 */
struct vec4 {
    float x, y, z, w;
    vec4(float X = 0, float Y = 0, float Z = 0, float W = 0) : x(X), y(Y), z(Z), w(W) {}

    vec4 operator+(vec4 const& v) const noexcept;
    vec4 operator-(vec4 const& v) const noexcept;
    vec4 operator*(float s) const noexcept;
    vec4 operator/(float s) const noexcept;

    vec4& operator+=(vec4 const& v) noexcept;
    vec4& operator-=(vec4 const& v) noexcept;
    vec4& operator*=(float s) noexcept;
    vec4& operator/=(float s) noexcept;

    float dot(vec4 const& v) const noexcept;
    float length() const noexcept;
    vec4 normalized() const noexcept;
};

// ---------------- mat3 ----------------
/**
 * @brief 3x3 column-major matrix for 2D transformations
 */
struct mat3 {
    float m[9]; // column-major

    mat3();
    static mat3 identity();
    static mat3 translate(vec2 const& v);
    static mat3 rotate(float radians);
    static mat3 scale(vec2 const& v);

    mat3 operator*(mat3 const& rhs) const noexcept;
    vec2 operator*(vec2 const& v) const noexcept;
};

// ---------------- mat4 ----------------
/**
 * @brief 4x4 column-major matrix for 3D transformations
 */
struct mat4 {
    float m[16]; // column-major

    mat4();
    static mat4 identity();
    static mat4 translate(vec3 const& v);
    static mat4 scale(vec3 const& v);
    static mat4 rotateX(float radians);
    static mat4 rotateY(float radians);
    static mat4 rotateZ(float radians);

    mat4 operator*(mat4 const& rhs) const noexcept;

    /** @brief Transform a vector (assumes w=1)
 *  @return Transformed vec3 */
    vec3 operator*(vec3 const& v) const noexcept; // assume w=1
};

#endif // MATHLIB_H
