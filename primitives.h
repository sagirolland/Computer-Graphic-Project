#if !defined(PRIMITIVES_HEADER)
#define PRIMITIVES_HEADER

#include "model.h"
#include "stb_ds.h"

// TODO(student): Create primitives here
typedef union triangle
{
    struct
    {
        Vec3 v0, v1, v2;
    };
    Vec3 vertices[3];
} triangle;
typedef union cube
{
    struct
    {
        Vec3 v0, v1, v2, v3, v4, v5, v6, v7;
    };
    Vec3 vertices[8];
} cube;
typedef union pyramid
{
    struct
    {
        Vec3 v0, v1, v2, v3, v4;
    };
    Vec3 vertices[5];
} pyramid;

struct sphere
{
    Vec3 center;
    float radius;
};

struct Model MakePrimitiveTriangle()
{
    Model model = {0};
    arrput(model.vertices, ((Vec3){-1.f, -1.f, 0.f}));
    arrput(model.vertices, ((Vec3){1.f, -1.f, 0.f}));
    arrput(model.vertices, ((Vec3){0.f, 1.f, 0.f}));
    model.vertexCount = arrlen(model.vertices);
    arrsetlen(model.normals, model.vertexCount);
    Vec3 v0 = model.vertices[0];
    Vec3 v1 = model.vertices[1];
    Vec3 v2 = model.vertices[2];
    Vec3 normal = V3Normalize(V3Cross(V3Sub(v1, v0), V3Sub(v2, v0)));
    for (int i = 0; i < model.vertexCount; i++)
    {
        model.normals[i] = normal;
    }
    return model;
}
struct Model MakePrimitiveCube()
{
    Model model = {0};
    const Vec3 corners[] = {
        {-1.f, -1.f, -1.f},
        {1.f, -1.f, -1.f},
        {1.f, 1.f, -1.f},
        {-1.f, 1.f, -1.f},
        {-1.f, -1.f, 1.f},
        {1.f, -1.f, 1.f},
        {1.f, 1.f, 1.f},
        {-1.f, 1.f, 1.f},
    };
    const int triangleCorners[] = {
        0, 2, 1, 0, 3, 2, // back
        4, 5, 6, 4, 6, 7, // front
        0, 4, 7, 0, 7, 3, // left
        1, 2, 6, 1, 6, 5, // right
        0, 1, 5, 0, 5, 4, // bottom
        3, 7, 6, 3, 6, 2  // top
    };
    for (int i = 0; i < sizeof(triangleCorners) / sizeof(triangleCorners[0]); ++i)
    {
        arrput(model.vertices, corners[triangleCorners[i]]);
    }
    model.vertexCount = arrlen(model.vertices);
    arrsetlen(model.normals, model.vertexCount);

    for (int i = 0; i < model.vertexCount; i += 3)
    {
        Vec3 v0 = model.vertices[i + 0];
        Vec3 v1 = model.vertices[i + 1];
        Vec3 v2 = model.vertices[i + 2];
        Vec3 normal = V3Normalize(V3Cross(V3Sub(v1, v0), V3Sub(v2, v0)));
        model.normals[i + 0] = normal;
        model.normals[i + 1] = normal;
        model.normals[i + 2] = normal;
    }

    return model;
}
static Vec3 SphereNormal(float theta, float phi)
{
    return (Vec3){
        sinf(theta) * cosf(phi),
        cosf(theta),
        sinf(theta) * sinf(phi)};
}
static Vec3 SpherePoint(Vec3 center, float radius, float theta, float phi)
{
    return (Vec3){
        center.x + radius * sinf(theta) * cosf(phi),
        center.y + radius * cosf(theta),
        center.z + radius * sinf(theta) * sinf(phi)};
}
static void AddTriangle(Model *model, Vec3 v0, Vec3 n0, Vec3 v1, Vec3 n1, Vec3 v2, Vec3 n2)
{
    arrput(model->vertices, v0);
    arrput(model->vertices, v1);
    arrput(model->vertices, v2);

    arrput(model->normals, n0);
    arrput(model->normals, n1);
    arrput(model->normals, n2);

    model->vertexCount += 3;
}

struct Model MakePrimitiveSphere()
{        
    Model model = {0};
    const int latitudeSegments = 12;
    const int longitudeSegments = 24;
    const float pi = 3.14159265f;
    const Vec3 center = {0.f, 0.f, 0.f};
    const float radius = 1.f;

    for (int i = 0; i < latitudeSegments; ++i)
    {
        float theta0 = pi * i / latitudeSegments;
        float theta1 = pi * (i + 1) / latitudeSegments;

        for (int j = 0; j < longitudeSegments; ++j)
        {
            float phi0 = 2.f * pi * j / longitudeSegments;
            float phi1 = 2.f * pi * (j + 1) / longitudeSegments;

            Vec3 a = SpherePoint(center, radius, theta0, phi0);
            Vec3 b = SpherePoint(center, radius, theta1, phi0);
            Vec3 c = SpherePoint(center, radius, theta1, phi1);
            Vec3 d = SpherePoint(center, radius, theta0, phi1);

            Vec3 na = SphereNormal(theta0, phi0);
            Vec3 nb = SphereNormal(theta1, phi0);
            Vec3 nc = SphereNormal(theta1, phi1);
            Vec3 nd = SphereNormal(theta0, phi1);

            if (i > 0)
            {
                AddTriangle(&model, a, na, d, nd, b, nb);
            }
            if (i < latitudeSegments - 1)
            {
                AddTriangle(&model, d, nd, c, nc, b, nb);
            }
        }
    }

    model.vertexCount = arrlen(model.vertices);
    return model;
}
struct Model MakePrimitive4SidedPyramid()
{
    Model model = {0};
    const Vec3 corners[] = {
        {-1.f, -1.f, -1.f},
        {1.f, -1.f, -1.f},
        {1.f, 1.f, -1.f},
        {-1.f, 1.f, -1.f},
        {0.f, 0.f, 1.f},
    };
    const int triangleCorners[] = {
        0, 1, 4,         // back
        1, 2, 4,         // right
        2, 3, 4,         // front
        3, 0, 4,         // left
        0, 3, 2, 0, 2, 1 // bottom
    };
    for (int i = 0; i < sizeof(triangleCorners) / sizeof(triangleCorners[0]); ++i)
    {
        arrput(model.vertices, corners[triangleCorners[i]]);
    }
    model.vertexCount = arrlen(model.vertices);
    arrsetlen(model.normals, model.vertexCount);

    for (int i = 0; i < model.vertexCount; i += 3)
    {
        Vec3 v0 = model.vertices[i + 0];
        Vec3 v1 = model.vertices[i + 1];
        Vec3 v2 = model.vertices[i + 2];
        Vec3 normal = V3Normalize(V3Cross(V3Sub(v1, v0), V3Sub(v2, v0)));
        model.normals[i + 0] = normal;
        model.normals[i + 1] = normal;
        model.normals[i + 2] = normal;
    }
    return model;
}

#endif // PRIMITIVES_HEADER
