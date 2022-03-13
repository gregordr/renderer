#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <iostream>

#include "readPng.cpp"

#include "bvh/triangle.hpp"
#include "bvh/ray.hpp"
#include "bvh/primitive_intersectors.hpp"
#include "bvh/bvh.hpp"
#include "bvh/vector.hpp"
#include "bvh/sweep_sah_builder.hpp"
#include "bvh/single_ray_traverser.hpp"

using BvhScalar = float;
using BvhVector3 = bvh::Vector3<BvhScalar>;
using BvhRay = bvh::Ray<BvhScalar>;
using BvhTriangle = bvh::Triangle<BvhScalar>;
using Bvh = bvh::Bvh<BvhScalar>;

class Triangle;

struct Point
{
    Point();
    Point(float _x, float _y, float _z);

    Point operator+(const Point &other) const;
    Point &operator+=(const Point &other);
    Point operator-(const Point &other) const;
    Point operator*(const float) const;
    float operator*(const Point &other) const;
    Point operator&(const Point &other) const;
    Point operator/(const float) const;
    Point rotateByX(const float) const;
    Point rotateByY(const float) const;
    Point rotateByZ(const float) const;

    Point normal() const;

    operator BvhVector3() const;

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

Color operator*(const Color &lhs, const float s);
Color operator*(const float s, const Color &lhs);
Color operator*(const Color &lhs, const int s);
Color operator*(const int s, const Color &lhs);

struct Ray
{
    Ray(const Point &origin, const Point &direction);

    Color findColor(const std::vector<Triangle> &triangles, size_t depth = 0, bool hitProvided = false, std::optional<bvh::ClosestPrimitiveIntersector<Bvh, BvhTriangle>::Result> &&hit = std::nullopt);

    Point origin;
    Point unitDir;

    operator BvhRay() const;
};

struct Intersection
{
    Ray ray;
    double hitDistance;
    Color color;

    Intersection(const Ray &_ray, double _hitDistance, const Color &_color) : ray(_ray), hitDistance(_hitDistance), color(_color) {}
    Intersection() : ray({Point(0, 0, 0), Point(0, 0, 1)}), hitDistance(0), color() {}
};

struct TriangleVertex
{
    struct VertexTexture
    {
        VertexTexture(float _u, float _v) : u(_u), v(_v) {}

        VertexTexture operator*(float val) const
        {
            return {u * val, v * val};
        }
        VertexTexture operator+(const VertexTexture &other) const
        {
            return {u + other.u, v + other.v};
        }

        float u;
        float v;
    };

    TriangleVertex(Point _v, VertexTexture _vt, Point _normal) : v(_v), vt(_vt), normal(_normal) {}

    TriangleVertex operator+(const Point &other) const
    {
        return {v + other, vt, normal};
    }

    TriangleVertex operator*(float val) const
    {
        return {v * val, vt * val, normal};
    }

    TriangleVertex operator+(const TriangleVertex &other) const
    {
        return {v + other.v, vt + other.vt, normal};
    }

    Point v;
    Point normal;
    VertexTexture vt;
};

class Triangle;

using ColorFunction = std::function<Color(const Triangle &, const TriangleVertex::VertexTexture &vt, png_bytep *, std::function<Color(Ray, int)> rec, const Ray &ray, float t, float u, float v, int depth)>;

class Triangle
{
public:
    Triangle(TriangleVertex _v1,
             TriangleVertex _v2,
             TriangleVertex _v3, ColorFunction _colorFunction) : v1(_v1), v2(_v2), v3(_v3), colorFunction(_colorFunction)
    {
    }

    Triangle operator+(const Point &other) const
    {
        return {v1 + other, v2 + other, v3 + other, colorFunction};
    }

    Triangle rotate(float a, float b, float c) const
    {
        return {{v1.v.rotateByX(a).rotateByY(b).rotateByZ(c), v1.vt, v1.normal.rotateByX(a).rotateByY(b).rotateByZ(c)}, {v2.v.rotateByX(a).rotateByY(b).rotateByZ(c), v2.vt, v2.normal.rotateByX(a).rotateByY(b).rotateByZ(c)}, {v3.v.rotateByX(a).rotateByY(b).rotateByZ(c), v3.vt, v3.normal.rotateByX(a).rotateByY(b).rotateByZ(c)}, colorFunction};
    }

    Color intersect(const Ray &ray, const std::vector<Triangle> &triangles, size_t depth, float t, float u, float v, std::function<Color(Ray, int)> rec) const
    {
        auto hitPoint = (v2 * u + v3 * v + v1 * (1 - u - v));
        return colorFunction(*this, hitPoint.vt, row_pointers, rec, ray, t, u, v, depth);
    }

    TriangleVertex v1;
    TriangleVertex v2;
    TriangleVertex v3;

private:
    Color getColor(const TriangleVertex::VertexTexture &vt) const
    {
        uint row = read_height - 1 - read_height * std::clamp(vt.v, 0.0f, 0.99f);
        uint col = read_width * std::clamp(vt.u, 0.0f, 0.99f);
        png_bytep rowVals = row_pointers[row];
        return Color{rowVals[col * 4 + 0], rowVals[col * 4 + 1], rowVals[col * 4 + 2]};
    }

    ColorFunction colorFunction;
};