
#include <cmath>
#include <vector>
#include <memory>
#include <execution>

#include "common.hpp"

Point::Point(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

Point Point::operator+(const Point &other) const
{
    return {x + other.x, y + other.y, z + other.z};
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

Color::Color(int _r, int _g, int _b) : r(_r), g(_g), b(_b) {}
Color::Color() : r(0), g(0), b(0) {}

Ray::Ray(const Point &_orig, const Point &_dir) : origin(_orig), unitDir(_dir / std::sqrt(_dir * _dir)) {}

Color Ray::findColor(const std::vector<std::shared_ptr<Triangle>> &triangles, size_t depth)
{
    std::vector<float> hitDistances(triangles.size());

    std::transform(std::execution::par_unseq, triangles.begin(), triangles.end(), hitDistances.begin(), [&](const std::shared_ptr<Triangle> &triangle)
                   { return triangle->intersectionDistance(*this, triangles); });

    auto closestItr = std::min_element(hitDistances.begin(), hitDistances.end(), [&](auto t1, auto t2)
                                       { return t1 < t2; }) -
                      hitDistances.begin() + triangles.begin();

    return closestItr->get()->intersect(*this, triangles);
}