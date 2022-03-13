
#include <vector>
#include <execution>

#include "objLoader.cpp"
#include "common.hpp"

class RayTracer
{
public:
    RayTracer(Point _origin, Point _dir, int _h, int _w) : origin(_origin), dir(_dir), h(_h), w(_w) {}

    void addTriangles(const std::vector<Triangle> &_triangles)
    {
        triangles.insert(triangles.begin(), _triangles.begin(), _triangles.end());
    }

    std::vector<Color> render() const
    {
        auto rays = generateRays();

        std::vector<BvhTriangle> bvhTriangles(triangles.size());
        std::vector<Color> colors(rays.size());

        std::transform(triangles.begin(), triangles.end(), bvhTriangles.begin(), [&](const auto &triangle)
                       { return BvhTriangle(triangle.v1.v, triangle.v2.v, triangle.v3.v); });

        auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(bvhTriangles.data(), bvhTriangles.size());
        auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(), bvhTriangles.size());

        Bvh bvh;
        bvh::SweepSahBuilder<Bvh> builder(bvh);
        builder.build(global_bbox, bboxes.get(), centers.get(), bvhTriangles.size());

        bvh::ClosestPrimitiveIntersector<Bvh, BvhTriangle> primitive_intersector(bvh, bvhTriangles.data());
        bvh::SingleRayTraverser<Bvh> traverser(bvh);

        std::function<Color(Ray, int)> fr = [&](Ray ray, int depth)
        { return getRayColor(ray, depth, primitive_intersector, traverser, fr); };

        std::transform(std::execution::par_unseq, rays.begin(), rays.end(), colors.begin(), [&](auto &ray)
                       { return fr(ray, 0); });

        return colors;
    }

private:
    Color getRayColor(Ray ray, int depth, bvh::ClosestPrimitiveIntersector<Bvh, BvhTriangle> primitive_intersector, bvh::SingleRayTraverser<Bvh> traverser, std::function<Color(Ray, int)> rec) const
    {
        auto hit = traverser.traverse(ray, primitive_intersector);

        if (hit)
        {
            return triangles.at(hit->primitive_index).intersect(ray, triangles, depth, hit->intersection.distance(), hit->intersection.u, hit->intersection.v, rec);
        }

        return Color{static_cast<int>(255 * ray.unitDir.y * 0.5f + 255 * 0.5f), static_cast<int>(255 * 0.7f + ray.unitDir.y * 255 * 0.3f), 255};
    }

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
    std::vector<Triangle> triangles;

    Point origin;
    Point dir;
    int h;
    int w;
};