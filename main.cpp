#include "mirror.cpp"
#include <iostream>
#include <png.h>
#include <cmath>

int writeImage(const char *filename, int width, int height, const std::vector<Color> &buffer, char *title)
{
    int code = 0;
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    // Open file for writing (binary mode)
    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        code = 1;
        goto finalise;
    }

    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        fprintf(stderr, "Could not allocate write struct\n");
        code = 1;
        goto finalise;
    }

    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        fprintf(stderr, "Could not allocate info struct\n");
        code = 1;
        goto finalise;
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Error during png creation\n");
        code = 1;
        goto finalise;
    }

    png_init_io(png_ptr, fp);

    // Write header (8 bit colour depth)
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Set title
    if (title != NULL)
    {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = title;
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    // Allocate memory for one row (3 bytes per pixel - RGB)
    row = (png_bytep)malloc(3 * width * sizeof(png_byte));

    // Write image data
    int x, y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            row[x * 3] = buffer[y * width + x].r;
            row[x * 3 + 1] = buffer[y * width + x].g;
            row[x * 3 + 2] = buffer[y * width + x].b;
        }
        png_write_row(png_ptr, row);
    }

    // End write
    png_write_end(png_ptr, NULL);

finalise:
    if (fp != NULL)
        fclose(fp);
    if (info_ptr != NULL)
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL)
        png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL)
        free(row);

    return code;
}

int main(int argc, char const *argv[])
{
    constexpr int dim = 400;

    read_png_file("spot/spot_texture.png");

    Obj textureCow("spot/spot_triangulated.obj", [&](const TextureTriangle &tr, const TriangleVertex::VertexTexture &vt, png_bytep *, const std::vector<std::shared_ptr<Triangle>> &triangles, const Ray &ray, float t, int depth = 0)
                   {
                       uint row = read_height - 1 - read_height * std::clamp(vt.v, 0.0f, 0.99f);
                       uint col = read_width * std::clamp(vt.u, 0.0f, 0.99f);
                       png_bytep rowVals = row_pointers[row];
                       return Color{rowVals[col * 4 + 0], rowVals[col * 4 + 1], rowVals[col * 4 + 2]};
                   });

    Obj rTextureCow("spot/spot_triangulated.obj", [&](const TextureTriangle &tr, const TriangleVertex::VertexTexture &vt, png_bytep *, const std::vector<std::shared_ptr<Triangle>> &triangles, const Ray &ray, float t, int depth = 0)
                    {
                        uint row = read_height - 1 - read_height * std::clamp(vt.v, 0.0f, 0.99f);
                        uint col = read_width * std::clamp(vt.u, 0.0f, 0.99f);
                        png_bytep rowVals = row_pointers[row];
                        float sum = (ray.origin + ray.unitDir * t) * Point(1, 1, 1) * 20;

                        return Color{rowVals[col * 4 + 0] / 2 + static_cast<int>(126 * std::sin(sum)), rowVals[col * 4 + 1] / 2 + static_cast<int>(126 * std::sin(sum)), rowVals[col * 4 + 2] / 2 + static_cast<int>(126 * std::sin(sum))};
                    });

    Obj mirrowCow("spot/spot_triangulated.obj", [&](const TextureTriangle &tr, const TriangleVertex::VertexTexture &vt, png_bytep *, const std::vector<std::shared_ptr<Triangle>> &triangles, const Ray &ray, float t, int depth = 0)
                  {
                      const auto normal = (tr.v1.v - tr.v2.v) & (tr.v1.v - tr.v3.v);
                      return Ray{ray.origin + ray.unitDir * t, ray.unitDir - (normal * (ray.unitDir * 2 * normal) / (normal * normal))}
                          .findColor(triangles, depth + 1);
                  });

    RayTracer tracer(Point{0, 0, 0}, Point{0, 0, 3}, dim, dim);

    for (int i = 0; i < 1; i++)
    {
        auto triangles = textureCow.setDisplacement(0, 0, 1.5).setRotation(3.1415 * i / 50, 3.1415 * i / 50, 0).getTriangles();
        auto mirrorTriangles = mirrowCow.setDisplacement(1, 0, 1.5).setRotation(3.1415 * i / 50, 3.1415 * i / 50, 0).getTriangles();
        auto rtTriangles = rTextureCow.setDisplacement(1, 0, 1.5).setRotation(3.1415 * i / 50, 3.1415 * i / 50, 0).getTriangles();
        // triangles.insert(triangles.begin(), mirrorTriangles.begin(), mirrorTriangles.end());
        triangles.insert(triangles.begin(), rtTriangles.begin(), rtTriangles.end());
        triangles.emplace_back(std::make_shared<Mirror>(Point{1, 0, 5}, Point{-0.5, 0.2, 1}));

        std::cout << "rendering" << std::endl;
        auto colors = tracer.render(triangles);
        std::string name = "out/cow" + std::to_string(i) + ".png";
        writeImage(name.c_str(), dim, dim, colors, "cow");
    }

    return 0;
}
