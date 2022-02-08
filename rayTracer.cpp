
#include <vector>
#include <execution>

#include "objLoader.cpp"
#include "common.hpp"

class RayTracer
{
public:
    RayTracer(Point _origin, Point _dir, int _h, int _w) : origin(_origin), dir(_dir), h(_h), w(_w) {}

    std::vector<Color> render(const std::vector<std::shared_ptr<Triangle>> &triangles) const
    {
        auto rays = generateRays();

        std::vector<Color> colors(rays.size());

        std::transform(std::execution::par_unseq, rays.begin(), rays.end(), colors.begin(), [&](auto &ray)
                       { return ray.findColor(triangles); });

        return colors;
    }

private:
    std::vector<Ray> generateRays() const
    {
        std::vector<Ray> rays;
        for (size_t i = 0; i < h; i++)
            for (size_t j = 0; j < w; j++)
            {
                // TODO: consider direction
                rays.emplace_back(origin, Point{-1 + 2.0f * j / w, +1 - 2.0f * i / h, +1});
            }

        return rays;
    }
    Point origin;
    Point dir;
    int h;
    int w;
};