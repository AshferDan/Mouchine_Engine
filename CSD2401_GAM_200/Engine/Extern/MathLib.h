#ifndef MATH_LIB_H
#define MATH_LIB_H	

#include <cmath>
#include <cstdint>

//----------------- vec2 -----------------//
struct vec2 {
	float x, y;
	vec2();
	vec2(float px, float py);

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

//----------------- vec3 -----------------//
struct vec3 {
	float x, y, z;
	vec3();
	vec3(float px, float py, float pz);

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

//----------------- vec4 -----------------//
struct vec4 {
	float x, y, z, w;
	vec4();
	vec4(float px, float py, float pz, float pw);

	vec4 operator+(vec4 const& v) const noexcept;
	vec4 operator-(vec4 const& v) const noexcept;
	vec4 operator*(float s) const noexcept;
	vec4 operator/(float s) const noexcept;
};

//----------------- mat3 -----------------//
struct mat3 {
	float m[9];

	mat3();
	static mat3 translate(vec2 const& v);
	static mat3 rotate(float radians);
	static mat3 scale(vec2 const& v);

	mat3 operator*(mat3 const& o) const;
};

//----------------- mat4 -----------------//
struct mat4 {
	float m[16];

	mat4();
	static mat4 translate(vec3 const& v);
	static mat4 scale(vec3 const& v);
	static mat4 rotateX(float radians);
	static mat4 rotateY(float radians);
	static mat4 rotateZ(float radians);

	mat4 operator*(mat4 const& o) const;
};

#endif // MATH_LIB