#include "MathLib.h"

//----------------- vec2 -----------------//
vec2::vec2() : x(0), y(0) {}
vec2::vec2(float px, float py) : x(px), y(py) {}

vec2 vec2::operator+(vec2 const& v) const noexcept { return vec2(x + v.x, y + v.y); }
vec2 vec2::operator-(vec2 const& v) const noexcept { return vec2(x - v.x, y - v.y); }
vec2 vec2::operator*(float s) const noexcept { return vec2(x * s, y * s); }
vec2 vec2::operator/(float s) const noexcept { return vec2(x / s, y / s); }

vec2& vec2::operator+=(vec2 const& v) noexcept { x += v.x; y += v.y; return *this; }
vec2& vec2::operator-=(vec2 const& v) noexcept { x -= v.x; y -= v.y; return *this; }
vec2& vec2::operator*=(float s) noexcept { x *= s; y *= s; return *this; }
vec2& vec2::operator/=(float s) noexcept { x /= s; y /= s; return *this; }

float vec2::dot(vec2 const& v) const noexcept { return x * v.x + y * v.y; }
float vec2::cross(vec2 const& v) const noexcept { return x * v.y - y * v.x; }
float vec2::length() const noexcept { return std::sqrt(x * x + y * y); }
vec2 vec2::normalized() const noexcept {
    float len = length();
    return (len > 0) ? (*this) / len : vec2(0, 0);
}
float vec2::angle() const noexcept { return std::atan2(y, x); }

//----------------- vec3 -----------------//
vec3::vec3() : x(0), y(0), z(0) {}
vec3::vec3(float px, float py, float pz) : x(px), y(py), z(pz) {}

vec3 vec3::operator+(vec3 const& v) const noexcept { return vec3(x + v.x, y + v.y, z + v.z); }
vec3 vec3::operator-(vec3 const& v) const noexcept { return vec3(x - v.x, y - v.y, z - v.z); }
vec3 vec3::operator*(float s) const noexcept { return vec3(x * s, y * s, z * s); }
vec3 vec3::operator/(float s) const noexcept { return vec3(x / s, y / s, z / s); }

vec3& vec3::operator+=(vec3 const& v) noexcept { x += v.x; y += v.y; z += v.z; return *this; }
vec3& vec3::operator-=(vec3 const& v) noexcept { x -= v.x; y -= v.y; z -= v.z; return *this; }
vec3& vec3::operator*=(float s) noexcept { x *= s; y *= s; z *= s; return *this; }
vec3& vec3::operator/=(float s) noexcept { x /= s; y /= s; z /= s; return *this; }

float vec3::dot(vec3 const& v) const noexcept { return x * v.x + y * v.y + z * v.z; }
vec3 vec3::cross(vec3 const& v) const noexcept {
    return vec3(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}
float vec3::length() const noexcept { return std::sqrt(x * x + y * y + z * z); }
vec3 vec3::normalized() const noexcept {
    float len = length();
    return (len > 0) ? (*this) / len : vec3(0, 0, 0);
}

//----------------- vec4 -----------------//
vec4::vec4() : x(0), y(0), z(0), w(0) {}
vec4::vec4(float px, float py, float pz, float pw) : x(px), y(py), z(pz), w(pw) {}

vec4 vec4::operator+(vec4 const& v) const noexcept { return vec4(x + v.x, y + v.y, z + v.z, w + v.w); }
vec4 vec4::operator-(vec4 const& v) const noexcept { return vec4(x - v.x, y - v.y, z - v.z, w - v.w); }
vec4 vec4::operator*(float s) const noexcept { return vec4(x * s, y * s, z * s, w * s); }
vec4 vec4::operator/(float s) const noexcept { return vec4(x / s, y / s, z / s, w / s); }

//----------------- mat3 -----------------//
mat3::mat3() {
    for (int i = 0; i < 9; i++) {
        m[i] = (i % 4 == 0) ? 1.f : 0.f;
    }
}

mat3 mat3::translate(vec2 const& v) {
    mat3 r;
    r.m[6] = v.x;
    r.m[7] = v.y;
    return r;
}

mat3 mat3::rotate(float radians) {
    mat3 r;
    float c = std::cos(radians);
    float s = std::sin(radians);

    r.m[0] = c; r.m[3] = s;
    r.m[1] = s; r.m[4] = c;
    return r;
}

mat3 mat3::scale(vec2 const& v) {
    mat3 r;
    r.m[0] = v.x;
    r.m[4] = v.y;
    return r;
}

mat3 mat3::operator*(mat3 const& o) const {
    mat3 r;
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            r.m[i * 3 + j] = m[0 * 3 + j] * o.m[i * 3 + 0] + m[1 * 3 + j] * o.m[i * 3 + 1] + m[2 * 3 + j] * o.m[i * 3 + 2];
            return r;
        }
    }
}

//----------------- mat4 -----------------//
mat4::mat4() {
    for (int i = 0; i < 16; ++i) {
        m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    }
}

mat4 mat4::translate(vec3 const& v) {
    mat4 r;
    r.m[12] = v.x;
    r.m[13] = v.y;
    r.m[14] = v.z;
    return r;
}

mat4 mat4::scale(vec3 const& v) {
    mat4 r;
    r.m[0] = v.x;
    r.m[5] = v.y;
    r.m[10] = v.z;
    return r;
}

mat4 mat4::rotateX(float rad) {
    mat4 r;
    float c = std::cos(rad), s = std::sin(rad);
    r.m[5] = c; r.m[9] = -s;
    r.m[6] = s; r.m[10] = c;
    return r;
}

mat4 mat4::rotateY(float rad) {
    mat4 r;
    float c = std::cos(rad), s = std::sin(rad);
    r.m[0] = c;  r.m[8] = s;
    r.m[2] = -s; r.m[10] = c;
    return r;
}

mat4 mat4::rotateZ(float rad) {
    mat4 r; float c = std::cos(rad), s = std::sin(rad);
    r.m[0] = c; r.m[4] = -s;
    r.m[1] = s; r.m[5] = c;
    return r;
}

mat4 mat4::operator*(mat4 const& o) const {
    mat4 r;
    for (int c = 0; c < 4; ++c)
        for (int rI = 0; rI < 4; ++rI)
            r.m[c * 4 + rI] = m[0 * 4 + rI] * o.m[c * 4 + 0] + m[1 * 4 + rI] * o.m[c * 4 + 1] + m[2 * 4 + rI] * o.m[c * 4 + 2] + m[3 * 4 + rI] * o.m[c * 4 + 3];
    return r;
}