#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>
#include <memory>
#include <unordered_map>

#include "common.hpp"

class Obj
{
public:
    Obj(std::string path, ColorFunction _colorFunction) : colorFunction(_colorFunction)
    {
        std::unordered_map<size_t, std::vector<size_t>> vertexToFaces;
        std::unordered_map<size_t, Point> vertexToNormal;
        std::vector<std::tuple<std::pair<int, int>, std::pair<int, int>, std::pair<int, int>>> f;

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

                vertexToFaces[v1 - 1].push_back(f.size());
                vertexToFaces[v2 - 1].push_back(f.size());
                vertexToFaces[v3 - 1].push_back(f.size());

                f.emplace_back(std::make_pair(v1 - 1, vt1 - 1), std::make_pair(v2 - 1, vt2 - 1), std::make_pair(v3 - 1, vt3 - 1));
            }
            else
            {
                throw "Invalid Obj file";
            }
        }

        // set normals
        for (const auto &[v, fcs] : vertexToFaces)
        {
            int n = fcs.size();
            for (const auto &fc : fcs)
            {
                const auto &v1 = vertices.at(std::get<0>(std::get<0>(f[fc])));
                const auto &v2 = vertices.at(std::get<0>(std::get<1>(f[fc])));
                const auto &v3 = vertices.at(std::get<0>(std::get<2>(f[fc])));
                auto normal = ((v2 - v1) & (v3 - v1)).normal();

                vertexToNormal[v] += normal / n;
            }
        }

        for (const auto &fc : f)
        {
            const auto &[vp1, vp2, vp3] = fc;
            const auto &[v1, vt1] = vp1;
            const auto &[v2, vt2] = vp2;
            const auto &[v3, vt3] = vp3;
            triangles.emplace_back(
                TriangleVertex{vertices.at(v1), vertexTextures.at(vt1), vertexToNormal.at(v1)},
                TriangleVertex{vertices.at(v2), vertexTextures.at(vt2), vertexToNormal.at(v2)}, TriangleVertex{vertices.at(v3), vertexTextures.at(vt3), vertexToNormal.at(v3)}, colorFunction);
        }
    }

    Obj
    setDisplacement(float x, float y, float z)
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

    std::vector<Triangle>
    getTriangles() const
    {
        std::vector<Triangle> output;
        std::transform(triangles.begin(), triangles.end(), std::back_inserter(output), [&](auto &triangle)
                       { return triangle.rotate(rotationX, rotationY, rotationZ) + displacement; });
        return output;
    }

private:
    ColorFunction colorFunction;
    std::vector<Point>
        vertices;
    std::vector<TriangleVertex::VertexTexture> vertexTextures;
    std::vector<Triangle> triangles;

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