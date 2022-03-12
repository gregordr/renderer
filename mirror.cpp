#include "rayTracer.cpp"

class Mirror : public Triangle
{
public:
    Mirror(Point _origin, Point _normal) : origin(_origin), normal(_normal) {}

    virtual Color intersect(const Ray &ray, const std::vector<std::shared_ptr<Triangle>> &triangles, size_t depth = 0) const
    {
        float denom = normal * (ray.unitDir);
        if (abs(denom) > 0.001f)
        {
            float t = (origin - ray.origin) * (normal) / denom;
            if (t >= 0.001f)
                return Ray{ray.origin + ray.unitDir * t, ray.unitDir - (normal * (ray.unitDir * 2 * normal) / (normal * normal))}
                    .findColor(triangles, depth + 1);
        }
        return Color{static_cast<int>(255 * ray.unitDir.y * 0.5f + 255 * 0.5f), static_cast<int>(255 * 0.7f + ray.unitDir.y * 255 * 0.3f), 255};
    }

    virtual float intersectionDistance(const Ray &ray, const std::vector<std::shared_ptr<Triangle>> &triangles, size_t depth = 0) const
    {
        float denom = normal * (ray.unitDir);
        if (abs(denom) > 0.001f)
        {
            float t = (origin - ray.origin) * (normal) / denom;
            if (t >= 0.001f)
                return t;
        }
        return std::numeric_limits<float>::infinity();
    }

private:
    Point origin;
    Point normal;
};
