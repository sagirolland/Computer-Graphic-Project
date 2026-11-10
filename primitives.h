#if !defined(PRIMITIVES_HEADER)
#define PRIMITIVES_HEADER

#include "model.h"
// TODO(student): Create primitives here
struct Model MakePrimitiveCube()
{
    Model model = {0};
    model.vertexCount = 8;
    if (model.vertexCount <= 0) return model;
    model.vertices = (Vec3*)malloc(sizeof(Vec3) * model.vertexCount);
    if(!model.vertices) {
        free(model.vertices);
        model.vertexCount = 0;
        return model;
    }
    model.normals = (Vec3*)malloc(sizeof(Vec3) * model.vertexCount);    
    if(!model.normals) {
        free(model.vertices);
        model.vertexCount = 0;
        return model;
    }

    model.vertices[0] = (Vec3){-1.f, -1.f, -1.f};
    model.vertices[1] = (Vec3){ 1.f, -1.f, -1.f};
    model.vertices[2] = (Vec3){ 1.f,  1.f, -1.f};
    model.vertices[3] = (Vec3){-1.f,  1.f, -1.f};
    model.vertices[4] = (Vec3){-1.f, -1.f,  1.f};
    model.vertices[5] = (Vec3){ 1.f, -1.f,  1.f};
    model.vertices[6] = (Vec3){ 1.f,  1.f,  1.f};
    model.vertices[7] = (Vec3){-1.f,  1.f,  1.f};
    
    return model;
}
struct Model MakePrimitiveSphere();
struct Model MakePrimitivePyramid();

#endif //PRIMITIVES_HEADER
