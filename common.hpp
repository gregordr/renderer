#pragma once

#include <vector>
#include <memory>

class Triangle;

struct Point
{
    Point(float _x, float _y, float _z);

    Point operator+(const Point &other) const;
    Point operator-(const Point &other) const;
    Point operator*(const float) const;
    float operator*(const Point &other) const;
    Point operator&(const Point &other) const;
    Point operator/(const float) const;
    Point rotateByX(const float) const;
    Point rotateByY(const float) const;
    Point rotateByZ(const float) const;

    float x;
    float y;
    float z;
};

struct Color
{

    Color(int r, int g, int b);
    Color();

    int r;
    int g;
    int b;
};

struct Ray
{
    Ray(const Point &origin, const Point &direction);

    Color findColor(const std::vector<std::shared_ptr<Triangle>> &triangles, size_t depth = 0);

    Point origin;
    Point unitDir;
};

struct Intersection
{
    Ray ray;
    double hitDistance;
    Color color;

    Intersection(const Ray &_ray, double _hitDistance, const Color &_color) : ray(_ray), hitDistance(_hitDistance), color(_color) {}
    Intersection() : ray({Point(0, 0, 0), Point(0, 0, 1)}), hitDistance(0), color() {}
};

class Triangle
{
public:
    virtual Color intersect(const Ray &ray, const std::vector<std::shared_ptr<Triangle>> &triangles, size_t depth = 0) const = 0;
    virtual float intersectionDistance(const Ray &ray, const std::vector<std::shared_ptr<Triangle>> &triangles, size_t depth = 0) const = 0;
};