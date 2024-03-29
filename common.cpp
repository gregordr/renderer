
#include <cmath>
#include <vector>
#include <memory>
#include <execution>

#include "common.hpp"

Point::Point() : x(0), y(0), z(0) {}
Point::Point(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

Point Point::operator+(const Point &other) const
{
    return {x + other.x, y + other.y, z + other.z};
}

Point &Point::operator+=(const Point &other)
{

    x += other.x;
    y += other.y;
    z += other.z;

    return *this;
    ;
}
Point Point::operator-(const Point &other) const
{
    return {x - other.x, y - other.y, z - other.z};
}
float Point::operator*(const Point &other) const
{
    return x * other.x + y * other.y + z * other.z;
}
Point Point::operator*(const float s) const
{
    return {x * s, y * s, z * s};
}
Point Point::operator/(const float s) const
{
    return {x / s, y / s, z / s};
}
Point Point::rotateByX(const float b) const
{
    return {x, y * std::cos(b) - z * std::sin(b), y * std::sin(b) + z * std::cos(b)};
}
Point Point::rotateByY(const float b) const
{
    return {x * std::cos(b) + z * std::sin(b), y, -x * std::sin(b) + z * std::cos(b)};
}
Point Point::rotateByZ(const float b) const
{
    return {x * std::cos(b) - y * std::sin(b), x * std::sin(b) + y * std::cos(b), z};
}

Point Point::operator&(const Point &other) const
{
    return Point{this->y * other.z - this->z * other.y, -this->x * other.z + this->z * other.x, this->x * other.y - this->y * other.x};
}

Point Point::normal() const
{
    return *this / std::sqrt(*this * *this);
}

Point::operator BvhVector3() const
{
    return {x, y, z};
}

Color::Color(int _r, int _g, int _b) : r(_r), g(_g), b(_b) {}
Color::Color() : r(0), g(0), b(0) {}

Color operator*(const Color &lhs, const float s)
{
    return {lhs.r * s, lhs.g * s, lhs.b * s};
}
Color operator*(const float s, const Color &lhs)
{
    return lhs * s;
}
Color operator*(const Color &lhs, const int s)
{
    return lhs * (float)s;
}
Color operator*(const int s, const Color &lhs)
{
    return lhs * s;
}

Ray::Ray(const Point &_orig, const Point &_dir) : origin(_orig), unitDir(_dir / std::sqrt(_dir * _dir)) {}

Ray::operator BvhRay() const
{
    return BvhRay(origin, unitDir, 0.01, 30000);
}