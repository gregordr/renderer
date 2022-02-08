#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <memory>

#include "common.hpp"
#include "readPng.cpp"

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

    TriangleVertex(Point _v, VertexTexture _vt) : v(_v), vt(_vt) {}

    TriangleVertex operator+(const Point &other) const
    {
        return {v + other, vt};
    }

    TriangleVertex operator*(float val) const
    {
        return {v * val, vt * val};
    }

    TriangleVertex operator+(const TriangleVertex &other) const
    {
        return {v + other.v, vt + other.vt};
    }

    Point v;
    VertexTexture vt;
};

class TextureTriangle;

using ColorFunction = std::function<Color(const TextureTriangle &, const TriangleVertex::VertexTexture &vt, png_bytep *, const std::vector<std::shared_ptr<Triangle>> &triangles, const Ray &ray, float t)>;

class TextureTriangle : public Triangle
{
public:
    TextureTriangle(TriangleVertex _v1,
                    TriangleVertex _v2,
                    TriangleVertex _v3, ColorFunction _colorFunction) : v1(_v1), v2(_v2), v3(_v3), colorFunction(_colorFunction) {}

    TextureTriangle operator+(const Point &other) const
    {
        return {v1 + other, v2 + other, v3 + other, colorFunction};
    }

    TextureTriangle rotate(float a, float b, float c) const
    {
        return {{v1.v.rotateByX(a).rotateByY(b).rotateByZ(c), v1.vt}, {v2.v.rotateByX(a).rotateByY(b).rotateByZ(c), v2.vt}, {v3.v.rotateByX(a).rotateByY(b).rotateByZ(c), v3.vt}, colorFunction};
    }

    Color intersect(const Ray &ray, const std::vector<std::shared_ptr<Triangle>> &triangles, size_t depth = 0) const
    {
        auto E1 = v2.v - v1.v;
        auto E2 = v3.v - v1.v;
        auto N = E1 & E2;
        float det = -(ray.unitDir * N);
        float invdet = 1.0 / det;
        auto AO = ray.origin - v1.v;
        auto DAO = AO & ray.unitDir;
        float u = E2 * DAO * invdet;
        float v = -(E1 * DAO * invdet);
        float t = AO * N * invdet;
        bool hit = (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);

        auto hitPoint = (v2 * u + v3 * v + v1 * (1 - u - v));
        return hit ? colorFunction(*this, hitPoint.vt, row_pointers, triangles, ray, t) : Color{static_cast<int>(255 * ray.unitDir.y * 0.5f + 255 * 0.5f), static_cast<int>(255 * 0.7f + ray.unitDir.y * 255 * 0.3f), 255};
    }

    float intersectionDistance(const Ray &ray, const std::vector<std::shared_ptr<Triangle>> &triangles, size_t depth = 0) const
    {
        auto E1 = v2.v - v1.v;
        auto E2 = v3.v - v1.v;
        auto N = E1 & E2;
        float det = -(ray.unitDir * N);
        float invdet = 1.0 / det;
        auto AO = ray.origin - v1.v;
        auto DAO = AO & ray.unitDir;
        float u = E2 * DAO * invdet;
        float v = -(E1 * DAO * invdet);
        float t = AO * N * invdet;
        bool hit = (det >= 1e-6 && t >= 0.0 && u >= 0.0 && v >= 0.0 && (u + v) <= 1.0);
        return hit ? t : std::numeric_limits<float>::infinity();
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

class Obj
{
public:
    Obj(std::string path, ColorFunction _colorFunction) : colorFunction(_colorFunction)
    {

        std::ifstream infile(path);

        for (std::string line; getline(infile, line);)
        {
            std::istringstream ss(line);
            std::string word;

            ss >> word;

            if (word == "v")
            {
                ss >> word;
                float x = std::stof(word);
                ss >> word;
                float y = std::stof(word);
                ss >> word;
                float z = std::stof(word);

                vertices.emplace_back(x, y, z);
            }
            else if (word == "vt")
            {
                ss >> word;
                float u = std::stof(word);
                ss >> word;
                float v = std::stof(word);

                vertexTextures.emplace_back(u, v);
            }
            else if (word == "f")
            {
                ss >> word;
                auto [v1, vt1] = split(word);
                ss >> word;
                auto [v2, vt2] = split(word);
                ss >> word;
                auto [v3, vt3] = split(word);

                triangles.emplace_back(TriangleVertex{vertices.at(v1 - 1), vertexTextures.at(vt1 - 1)}, TriangleVertex{vertices.at(v2 - 1), vertexTextures.at(vt2 - 1)}, TriangleVertex{vertices.at(v3 - 1), vertexTextures.at(vt3 - 1)}, colorFunction);
            }
            else
            {
                throw "Invalid Obj file";
            }
        }
    }

    Obj setDisplacement(float x, float y, float z)
    {
        displacement = {x, y, z};
        return *this;
    }

    Point getDisplacement() const
    {
        return displacement;
    }

    Obj setRotation(float x, float y, float z)
    {
        rotationX = x;
        rotationY = y;
        rotationZ = z;
        return *this;
    }

    std::tuple<float, float, float> getRotation() const
    {
        return {
            rotationX,
            rotationY,
            rotationZ,
        };
    }

    std::vector<std::shared_ptr<Triangle>>
    getTriangles() const
    {
        std::vector<std::shared_ptr<Triangle>> output;
        std::transform(triangles.begin(), triangles.end(), std::back_inserter(output), [&](auto &triangle)
                       { return std::make_shared<TextureTriangle>(triangle.rotate(rotationX, rotationY, rotationZ) + displacement); });
        return output;
    }

private:
    ColorFunction colorFunction;
    std::vector<Point>
        vertices;
    std::vector<TriangleVertex::VertexTexture> vertexTextures;
    std::vector<TextureTriangle> triangles;

    Point displacement = {0, 0, 0};
    float rotationX = 0;
    float rotationY = 0;
    float rotationZ = 0;

    std::pair<int, int> split(std::string inp)
    {
        int v1;
        int v2;

        std::stringstream ss(inp);

        std::string segment;
        std::getline(ss, segment, '/');
        v1 = stoi(segment);
        std::getline(ss, segment, '/');
        v2 = stoi(segment);

        return {v1, v2};
    }
};